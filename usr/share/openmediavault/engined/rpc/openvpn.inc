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
require_once "openmediavault/object.inc";
require_once "openmediavault/rpc.inc";
require_once "openmediavault/util.inc";

use Phar;
use PharData;

class OMVRpcServiceOpenVpn extends OMVRpcServiceAbstract
{
    private $ipPoolReservationsPath = "/etc/openvpn/ipp.txt";
    private $openVpnStatusLogPath = "/etc/openvpn/openvpn-status.log";

    private function getEventMessagePath()
    {
        return "org.openmediavault.services.openvpn";
    }

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
     * Get general settings.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        $object = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        $settingsObject["enable"]             = boolval($object["enable"]);
        $settingsObject["port"]               = intval($object["port"]);
        $settingsObject["protocol"]           = $object["protocol"];
        $settingsObject["compression"]        = boolval($object["compression"]);
        $settingsObject["pam_authentication"] = boolval($object["pam_authentication"]);
        $settingsObject["extra_options"]      = $object["extra_options"];
        $settingsObject["loglevel"]           = intval($object["loglevel"]);
        $settingsObject["vpn_network"]        = $object["vpn_network"];
        $settingsObject["vpn_mask"]           = $object["vpn_mask"];
        $settingsObject["default_gateway"]    = boolval($object["default_gateway"]);
        $settingsObject["client_to_client"]   = boolval($object["client_to_client"]);
        $settingsObject["dns"]                = $object["dns"];
        $settingsObject["dns_domains"]        = $object["dns_domains"];
        $settingsObject["wins"]               = $object["wins"];
        $settingsObject["public_address"]     = $object["public_address"];

        return $settingsObject;
    }

    /**
     * Set general settings.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

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

        // Prepare configuration data.
        $object = array(
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
        );

        // Get old configuration object.
        $oldConfig = $xmlConfig->get($this->getXpath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        $object["clients"] = $oldConfig["clients"];

        // Set configuration object.
        if (false === $xmlConfig->replace($this->getXpath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXpath()
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    /**
     * Get list of certificate configurations.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function getList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_EVERYONE
        ));

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

        // Get configuration object.
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

        $clients = array();

        foreach ($uuids as $uuid) {
            try {
                $clients[] = $this->get(array("uuid" => $uuid), $this->getAdminContext());
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
     * Get certificate configuration.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function get($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

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
     * Set certificate configuration.
     *
     * @param array $params
     * @param array $context
     *
     * @return array
     */
    public function set($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

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

        $client = array(
            "uuid" => empty($params["uuid"]) ? OMVUtil::uuid() : $params["uuid"],
            "associated_user" => $params["associated_user"],
            "common_name" => $params["common_name"],
        );

        // Save configuration.
        $success = false;
        $xpath = null;

        if (empty($params["uuid"])) {
            $success = $xmlConfig->set(
                $xpath = $this->getClientsXpath(),
                array("client" => $client),
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

        // Generate certificate.
        $cmd = sprintf(
            "export LANG=C; omv-mkconf %s add %s 2>&1",
            strtolower($this->getName()),
            $client["uuid"]
        );

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }

        return $client;
    }

    /**
     * Delete certificate configuration.
     *
     * @param array $params
     * @param array $context
     */
    public function delete($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

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
                implode("\n", $output)
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
     *
     * @return array
     */
    public function downloadCertificate($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_EVERYONE
        ));

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

        // Check if user is eligble to download certificate.
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

        $files = array(
            "$commonName-ca.crt" => sprintf("%s/ca.crt", $keyDir),
            "$commonName-client.crt" => sprintf("%s/%s.crt", $keyDir, $uuid),
            "$commonName-client.key" => sprintf("%s/%s.key", $keyDir, $uuid),
        );

        $stringContent = array(
            "$commonName-client.conf" => $configContent,
            "$commonName-client.ovpn" => $configContent,
        );

        foreach ($files as $filename => $file) {
            $archive->addFile($file, $filename);
        }

        foreach ($stringContent as $filename => $content) {
            $archive->addFromString($filename, $content);
        }

        return array(
            "contenttype" => "application/zip",
            "filename" => "openvpn-$commonName.zip",
            "filepath" => $tmpFile,
        );
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
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

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

        $data = gettext("-------OpenVPN status--------") . PHP_EOL
              . $status
              . PHP_EOL . PHP_EOL
              . gettext("-----IP pool reservation-----") . PHP_EOL
              . $reservations;

        return $data;
    }

    /**
     * Generate client configuration text.
     *
     * @param array $client Client certificate configuration.
     * @param array $settings The OpenVPN plugin configuration.
     *
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

        $conf = "";
        $conf .= "client\n";
        $conf .= "remote $publicAddress $port\n";
        $conf .= "proto $protocol\n";
        $conf .= "dev tun\n";
        $conf .= "ca   $commonName-ca.crt\n";
        $conf .= "cert $commonName-client.crt\n";
        $conf .= "key  $commonName-client.key\n";
        $conf .= "ns-cert-type server\n";
        $conf .= "$compression\n";
        $conf .= "$pam_authentication\n";
        $conf .= "persist-key\n";
        $conf .= "persist-tun\n";
        $conf .= "nobind\n";
        $conf .= "resolv-retry infinite\n";
        $conf .= "verb 3\n";
        $conf .= "mute 10\n";

        return $conf;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceOpenVpn());
