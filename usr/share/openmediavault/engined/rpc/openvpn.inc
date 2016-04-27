<?php

/**
 * Copyright (C) 2014 OpenMediaVault Plugin Developers.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

namespace OMV\Engined\Rpc;

use OMV\Config\ConfigObject;
use OMV\Config\Database;
use OMV\Config\DatabaseException;
use OMV\Engine\Notify\Dispatcher;
use OMV\Rpc\ServiceAbstract;
use OMV\System\Process;
use PharData;

class OpenVpn extends ServiceAbstract
{
    /** @var string */
    private $clientDataModelPath = 'conf.service.openvpn.client';

    /** @var Database */
    private $database;

    /** @var string */
    private $dataModelPath = 'conf.service.openvpn';

    /** @var Dispatcher */
    private $dispatcher;

    /* @var string */
    private $eventMessagePath = 'org.openmediavault.conf.service.openvpn';

    /* @var string */
    private $ipPoolReservationsPath = '/etc/openvpn/ipp.txt';

    /* @var string */
    private $openVpnStatusLogPath = '/var/log/openvpn-status.log';

    public function __construct(Database $database = null, Dispatcher $dispatcher = null)
    {
        $this->database = $database ?: Database::getInstance();
        $this->dispatcher = $dispatcher ?: Dispatcher::getInstance();
    }

    /**
     * Get the RPC service name.
     *
     * @return void
     */
    public function getName()
    {
        return 'OpenVpn';
    }

    /**
     * Initialize the RPC service.
     *
     * @return void
     */
    public function initialize()
    {
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');
        $this->registerMethod('getList');
        $this->registerMethod('get');
        $this->registerMethod('set');
        $this->registerMethod('delete');
        $this->registerMethod('downloadCertificate');
        $this->registerMethod('getStats');
    }

    /**
     * Get the general settings.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function getSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);

        $object = $this->database->get($this->dataModelPath);
        $object->remove('clients');

        return $object->getAssoc();
    }

    /**
     * Set the general settings.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function setSettings($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.openvpn.setsettings');

        $object = $this->database->get($this->dataModelPath);
        $object->setAssoc($params);

        $this->database->set($object);
        $this->dispatcher->notify(OMV_NOTIFY_MODIFY, $this->eventMessagePath, $object);

        return $object->getAssoc();
    }

    /**
     * Get a list of certificate configurations.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function getList($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_EVERYONE]);
        $this->validateMethodParams($params, 'rpc.common.getlist');

        $objects = [];

        if ($context === $this->getAdminContext()) {
            $objects = $this->database->get($this->clientDataModelPath);
        } else {
            try {
                $objects = $this->database->get($this->clientDataModelPath, 'associated_user', $context['username']);
            } catch (DatabaseException $e) {
                // OMV throws and exception when there was no match.
            }
        }

        // Make sure $objects always is an array.
        if (!is_array($objects)) {
            $objects = [$objects];
        }

        $clients = [];

        foreach ($objects as $object) {
            $clients[] = $object->getAssoc();
        }

        return $this->applyFilter(
            $clients,
            $params['start'],
            $params['limit'],
            $params['sortfield'],
            $params['sortdir']
        );
    }

    /**
     * Get single a certificate configuration.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function get($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.common.objectuuid');

        $object = $this->database->get($this->clientDataModelPath, $params['uuid']);

        return $object->getAssoc();
    }

    /**
     * Set a certificate configuration.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function set($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.openvpn.set');

        $object = new ConfigObject($this->clientDataModelPath);
        $object->setAssoc($params);

        if ($object->isNew()) {
            $this->database->assertIsUnique($object, 'common_name');
        }

        $this->database->set($object);

        // Generate the certificate.
        $cmd = new Process('omv-mkconf', 'openvpn', 'add', escapeshellarg($object->get('uuid')));
        $cmd->setRedirect2to1();
        $cmd->execute();

        return $object->getAssoc();
    }

    /**
     * Delete certificate configuration.
     *
     * @param array $params
     * @param array $context
     *
     * @return void
     */
    public function delete($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);
        $this->validateMethodParams($params, 'rpc.common.objectuuid');

        $object = $this->database->get($this->clientDataModelPath, $params['uuid']);

        // Revoke certificate and then delete from the configuration if successful.
        $cmd = new Process('omv-mkconf', 'openvpn', 'revoke', escapeshellarg($object->get('uuid')));
        $cmd->setRedirect2to1();
        $cmd->execute();

        $this->database->delete($object);
    }

    /**
     * Generate an archive including client certificates and configuration and
     * return the parameters needed to download the archive.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function downloadCertificate($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_EVERYONE]);
        $this->validateMethodParams($params, 'rpc.common.objectuuid');

        $object = $this->database->get($this->clientDataModelPath, $params['uuid']);
        $settings = $this->database->get($this->dataModelPath);

        $commonName = $object->get('common_name');

        // Check if the user is eligible to download the certificate.
        if ($context !== $this->getAdminContext() && $context['username'] !== $object->get('associated_user')) {
            return;
        }

        // Create a new temporary archive file.
        $tmpFile = sprintf('%s/%s.zip', sys_get_temp_dir(), uniqid());
        $archive = new PharData($tmpFile);

        $keyDir = '/etc/openvpn/keys';
        $ca = sprintf('%s/ca.crt', $keyDir);
        $cert = sprintf('%s/%s.crt', $keyDir, $commonName);
        $key = sprintf('%s/%s.key', $keyDir, $commonName);

        $files = [
            $commonName . '-ca.crt' => $ca,
            $commonName . '-client.crt' => $cert,
            $commonName . '-client.key' => $key,
        ];

        $stringContent = [
            $commonName . '-client.conf' => self::generateClientConf($object, $settings),
            $commonName . '-client.ovpn' => self::generateClientConf($object, $settings, $ca, $cert, $key),
        ];

        foreach ($files as $filename => $file) {
            $archive->addFile($file, $filename);
        }

        foreach ($stringContent as $filename => $content) {
            $archive->addFromString($filename, $content);
        }

        return [
            'contenttype' => 'application/zip',
            'filename' => 'openvpn-' . $commonName . '.zip',
            'filepath' => $tmpFile,
        ];
    }

    /**
     * Get general OpenVPN stats.
     *
     * @param array $params
     * @param array $context
     *
     * @return string
     */
    public function getStats($params, $context)
    {
        $this->validateMethodContext($context, ['role' => OMV_ROLE_ADMINISTRATOR]);

        $object = $this->database->get($this->dataModelPath);

        if (!$object->get('enable')) {
            return gettext('OpenVPN is disabled.');
        }

        $status = file_get_contents($this->openVpnStatusLogPath);

        if (!$status) {
            $status = gettext('No users connected.');
        }

        // Get IP pool reservation.
        $reservations = file_get_contents($this->ipPoolReservationsPath);

        if (!$reservations) {
            $reservations = gettext('No IP reserved.');
        }

        $data = implode(PHP_EOL, [
            gettext('------ OpenVPN status -------'),
            $status . PHP_EOL,
            gettext('---- IP pool reservation ----'),
            $reservations . PHP_EOL,
        ]);

        return $data;
    }

    /**
     * Generate client configuration text.
     *
     * @param ConfigObject $client   Client certificate configuration.
     * @param ConfigObject $settings The OpenVPN plugin configuration.
     * @param string       $ca       Path to the CA. If set the CA will be
     *                               included in the file.
     * @param string       $cert     Path to public key. If set the certificate
     *                               will be included in the file.
     * @param string       $key      Path to private key. If set the certificate
     *                               will be included in the file.
     *
     * @return string
     */
    private static function generateClientConf(
        ConfigObject $client,
        ConfigObject $settings,
        $ca = null,
        $cert = null,
        $key = null
    ) {
        $commonName = $client->get('common_name');

        $publicAddress = $settings->get('public_address');
        $port = $settings->get('port');
        $protocol = $settings->get('protocol');
        $compression = $settings->get('compression') ? 'comp-lzo' : ';comp-lzo';
        $pam_authentication = $settings->get('pam_authentication') ? 'auth-user-pass' : ';auth-user-pass';

        $configOptions = [
            'client',
            'remote ' . $publicAddress . ' ' . $port,
            'proto ' . $protocol,
            'dev tun',
            'ns-cert-type server',
            $compression,
            $pam_authentication,
            'persist-key',
            'persist-tun',
            'nobind',
            'resolv-retry infinite',
            'verb 3',
            'mute 10',
        ];

        if ($ca) {
            $configOptions[] = '<ca>';
            $configOptions[] = file_get_contents($ca);
            $configOptions[] = '</ca>';
        } else {
            $configOptions[] = 'ca   ' . $commonName . '-ca.crt';
        }

        if ($cert) {
            $configOptions[] = '<cert>';
            $configOptions[] = file_get_contents($cert);
            $configOptions[] = '</cert>';
        } else {
            $configOptions[] = 'cert ' . $commonName . '-client.crt';
        }

        if ($key) {
            $configOptions[] = '<key>';
            $configOptions[] = file_get_contents($key);
            $configOptions[] = '</key>';
        } else {
            $configOptions[] = 'key  ' . $commonName . '-client.key';
        }

        return implode(PHP_EOL, $configOptions);
    }
}
