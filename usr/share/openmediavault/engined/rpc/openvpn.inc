<?php

/**
 * Copyright (C) 2014 OpenMediaVault Plugin Developers
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

require_once "openmediavault/error.inc";
require_once "openmediavault/notify.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/util.inc";

use Phar;
use PharData;

class OMVRpcServiceOpenVpn extends OMVRpcServiceAbstract
{
    private $eventMessagePath = "org.openmediavault.services.openvpn";
    private $ipPoolReservationsPath = "/etc/openvpn/ipp.txt";
    private $openVpnStatusLogPath = "/etc/openvpn/openvpn-status.log";


    private function getXpath()
    {
        return "/config/services/openvpn";
    }

    private function getClientsXpath()
    {
        return sprintf("%s/clients", $this->getXpath(), $uuid);
    }

    private function getClientXpathByUuid($uuid)
    {
        return sprintf("%s/client[uuid='%s']", $this->getClientsXpath(), $uuid);
    }

    /**
     * Get the RPC service name.
     *
     * @return void
     */
    public function getName()
    {
        return "OpenVpn";
    }

    /**
     * Initialize the RPC service.
     *
     * @return void
     */
    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getList");
        $this->registerMethod("get");
        $this->registerMethod("set");
        $this->registerMethod("delete");
        $this->registerMethod("downloadCertificate");
        $this->registerMethod("getStats");
    }

    /**
     * Get the general settings.
     *
     * @param array $params
     * @param array $context
     * @return array
     *
     * @throws OMVException
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        $settingsObject["enable"] = boolval($object["enable"]);
        $settingsObject["port"] = intval($object["port"]);
        $settingsObject["protocol"] = $object["protocol"];
        $settingsObject["compression"] = boolval($object["compression"]);
        $settingsObject["pam_authentication"] = boolval($object["pam_authentication"]);
        $settingsObject["extra_options"] = $object["extra_options"];
        $settingsObject["loglevel"] = intval($object["loglevel"]);
        $settingsObject["vpn_network"] = $object["vpn_network"];
        $settingsObject["vpn_mask"] = $object["vpn_mask"];
        $settingsObject["default_gateway"] = boolval($object["default_gateway"]);
        $settingsObject["client_to_client"] = boolval($object["client_to_client"]);
        $settingsObject["dns"] = $object["dns"];
        $settingsObject["dns_domains"] = $object["dns_domains"];
        $settingsObject["wins"] = $object["wins"];
        $settingsObject["public_address"] = $object["public_address"];

        return $settingsObject;
    }

    /**
     * Set the general settings.
     *
     * @param array $params
     * @param array $context
     * @return array
     *
     * @throws OMVException
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "enable": { "type": "boolean" },
                    "port": { "type": "integer" },
                    "protocol": { "type": "string", "enum": ["udp", "tcp"] },
                    "compression": { "type": "boolean" },
                    "pam_authentication": { "type": "boolean" },
                    "extra_options": { "type": "string", "optional": true},
                    "loglevel": { "type": "integer", "enum": [0, 2, 5, 7] },
                    "vpn_network": { "type": "string" },
                    "vpn_mask": { "type": "string" },
                    "default_gateway": { "type": "boolean" },
                    "client_to_client": { "type": "boolean" },
                    "dns": { "type": "string", "optional": true },
                    "dns_domains": { "type": "string", "optional": true },
                    "wins": { "type": "string", "optional": true },
                    "public_address": { "type": "string" }
                }
            }'
        );

        // Prepare the configuration data.
        $object = [
            "enable" => boolval($params["enable"]),
            "port" => intval($params["port"]),
            "protocol" => $params["protocol"],
            "compression" => boolval($params["compression"]),
            "pam_authentication" => boolval($params["pam_authentication"]),
            "extra_options" => $params["extra_options"],
            "loglevel" => intval($params["loglevel"]),
            "vpn_network" => $params["vpn_network"],
            "vpn_mask" => $params["vpn_mask"],
            "default_gateway" => boolval($params["default_gateway"]),
            "client_to_client" => boolval($params["client_to_client"]),
            "dns" => $params["dns"],
            "dns_domains" => $params["dns_domains"],
            "wins" => $params["wins"],
            "public_address" => $params["public_address"],
        ];

        // Get the old configuration object.
        $oldConfig = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        $object["clients"] = $oldConfig["clients"];

        // Set the configuration object.
        if (false === $xmlConfig->replace($this->getXpath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        // Notify about the configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->eventMessagePath, $object);

        return $object;
    }

    /**
     * Get a list of certificate configurations.
     *
     * @param array $params
     * @param array $context
     * @return array
     *
     * @throws OMVException
     */
    public function getList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "start": { "type": "integer" },
                    "limit": {' . $GLOBALS["OMV_JSONSCHEMA_COUNTFIELD"] . '},
                    "sortfield": {' . $GLOBALS["OMV_JSONSCHEMA_SORTFIELD"] . '},
                    "sortdir": {' . $GLOBALS["OMV_JSONSCHEMA_SORTDIR"] . '}
                }
            }'
        );

        // Get the configuration object.
        $xpath = null;

        if ($context == $this->getAdminContext()) {
            $xpath = sprintf("%s/client/uuid", $this->getClientsXpath());
        } else {
            $xpath = sprintf(
                "%s/client[associated_user='%s']/uuid",
                $this->getClientsXpath(),
                $context["username"]
            );
        }

        $uuids = $xmlConfig->getList($xpath);

        if (is_null($uuids)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xpath
            );
        }

        $clients = [];

        foreach ($uuids as $uuid) {
            try {
                $clients[] = $this->get(["uuid" => $uuid], $this->getAdminContext());
            } catch (Exception $e) {
                throw $e;
            }
        }

        return $this->applyFilter(
            $clients,
            $params["start"],
            $params["limit"],
            $params["sortfield"],
            $params["sortdir"]
        );
    }

    /**
     * Get single a certificate configuration.
     *
     * @param array $params
     * @param array $context
     * @return array
     *
     * @throws OMVException
     */
    public function get($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "uuid": {' . $GLOBALS["OMV_JSONSCHEMA_UUID"] . '}
                }
            }'
        );

        $client = $xmlConfig->get($this->getClientXpathByUuid($params["uuid"]));

        if (is_null($client)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xpath
            );
        }

        return $client;
    }

    /**
     * Set a certificate configuration.
     *
     * @param array $params
     * @param array $context
     * @return array
     *
     * @throws OMVException
     */
    public function set($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "uuid": {' . $GLOBALS["OMV_JSONSCHEMA_UUID_UNDEFINED"] . '},
                    "associated_user": { "type": "string" },
                    "common_name": { "type": "string" }
                }
            }'
        );

        $client = [
            "uuid" => empty($params["uuid"]) ? OMVUtil::uuid() : $params["uuid"],
            "associated_user" => $params["associated_user"],
            "common_name" => $params["common_name"],
        ];

        // Save the configuration.
        $success = false;
        $xpath = null;

        if (empty($params["uuid"])) {
            $success = $xmlConfig->set(
                $xpath = $this->getClientsXpath(),
                ["client" => $client],
                $client
            );
        } else {
            $success = $xmlConfig->replace(
                $xpath = $this->getClientXpathByUuid($params["uuid"]),
                $client
            );
        }

        if (!$success) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
                $xpath
            );
        }

        // Generate the certificate.
        $cmd = sprintf(
            "export LANG=C; omv-mkconf %s add %s 2>&1",
            strtolower($this->getName()),
            $client["uuid"]
        );

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode(PHP_EOL, $output)
            );
        }

        return $client;
    }

    /**
     * Delete certificate configuration.
     *
     * @param array $params
     * @param array $context
     * @return void
     *
     * @throws OMVException
     */
    public function delete($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "uuid": {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '}
                }
            }'
        );

        // Revoke certificate and then delete from xml if successful.
        $cmd = sprintf(
            "export LANG=C; omv-mkconf %s revoke %s 2>&1",
            strtolower($this->getName()),
            $params["uuid"]
        );

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode(PHP_EOL, $output)
            );
        }

        $client = $xmlConfig->delete($this->getClientXpathByUuid(
            $params["uuid"]
        ));

        if (is_null($client)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
                $this->getClientXpathByUuid($params["uuid"])
            );
        }
    }

    /**
     * Generate an archive including client certificates and configuration and
     * return the parameters needed to download the archive.
     *
     * @param array $params
     * @param array $context
     * @return array
     *
     * @throws OMVException
     */
    public function downloadCertificate($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_EVERYONE
        ]);

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams(
            $params,
            '{
                "type": "object",
                "properties": {
                    "uuid": {' . $GLOBALS["OMV_JSONSCHEMA_UUID"] . '}
                }
            }'
        );

        $client = $this->get($params, $this->getAdminContext());
        $uuid = $client["uuid"];
        $commonName = $client["common_name"];

        // Check if the user is eligible to download the certificate.
        if ($context != $this->getAdminContext()) {
            if ($context["username"] != $client["associated_user"]) {
                return;
            }
        }

        $configContent = $this->generateClientConf(
            $client,
            $xmlConfig->get($this->getXpath())
        );

        // Create a new tempory archive file.
        $tmpFile = sprintf("%s/%s.zip", sys_get_temp_dir(), uniqid());
        $archive = new PharData($tmpFile);

        $keyDir = "/etc/openvpn/keys";

        $files = [
            "$commonName-ca.crt" => sprintf("%s/ca.crt", $keyDir),
            "$commonName-client.crt" => sprintf("%s/%s.crt", $keyDir, $uuid),
            "$commonName-client.key" => sprintf("%s/%s.key", $keyDir, $uuid),
        ];

        $stringContent = [
            "$commonName-client.conf" => $configContent,
            "$commonName-client.ovpn" => $configContent,
        ];

        foreach ($files as $filename => $file) {
            $archive->addFile($file, $filename);
        }

        foreach ($stringContent as $filename => $content) {
            $archive->addFromString($filename, $content);
        }

        return [
            "contenttype" => "application/zip",
            "filename" => "openvpn-$commonName.zip",
            "filepath" => $tmpFile,
        ];
    }

    /**
     * Get general OpenVPN stats.
     *
     * @param array $params
     * @param array $context
     * @return string
     *
     * @throws OMVException
     */
    public function getStats($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, [
            "role" => OMV_ROLE_ADMINISTRATOR
        ]);

        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        if (!boolval($object["enable"])) {
            return gettext("OpenVPN is disabled.");
        }

        // TODO: Switch file_get_contents to SplFileObject::fread (>PHP 5.5.11).
        // Get users currently logged in.
        $status = file_get_contents($this->openVpnStatusLogPath);

        if (!$status) {
            $status = gettext("No users connected.");
        }

        // Get IP pool reservation.
        $reservations = file_get_contents($this->ipPoolReservationsPath);

        if (!$reservations) {
            $reservations = gettext("No IP reserved.");
        }

        $data = gettext("-------OpenVPN status--------")
              . PHP_EOL
              . $status
              . PHP_EOL
              . PHP_EOL
              . gettext("-----IP pool reservation-----")
              . PHP_EOL
              . $reservations;

        return $data;
    }

    /**
     * Generate client configuration text.
     *
     * @param array $client Client certificate configuration.
     * @param array $settings The OpenVPN plugin configuration.
     * @return string
     */
    private function generateClientConf($client, $settings)
    {
        $commonName = $client["common_name"];
        $publicAddress = $settings["public_address"];
        $port = $settings["port"];
        $protocol = $settings["protocol"];
        $compression = empty($settings["compression"]) ? ";comp-lzo" : "comp-lzo";
        $pam_authentication = empty($settings["pam_authentication"]) ? ";auth-user-pass" : "auth-user-pass";

        $configOptions = [
            "client",
            "remote $publicAddress $port",
            "proto $protocol",
            "dev tun",
            "ca   $commonName-ca.crt",
            "cert $commonName-client.crt",
            "key  $commonName-client.key",
            "ns-cert-type server",
            $compression,
            $pam_authentication,
            "persist-key",
            "persist-tun",
            "nobind",
            "resolv-retry infinite",
            "verb 3",
            "mute 10",
        ];

        return implode(PHP_EOL, $configOptions);
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceOpenVpn());
