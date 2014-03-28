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

require_once "openmediavault/object.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/util.inc";
require_once "openmediavault/rpc.inc";
require_once "openmediavault/notify.inc";

class OMVRpcServiceOpenVPN extends OMVRpcServiceAbstract
{
    /* Helpers */

    private function getEventMessagePath()
    {
        return "org.openmediavault.services.openvpn";
    }

    private function getXPath()
    {
        return "/config/services/openvpn";
    }

    private function getClientsXPath()
    {
        return sprintf("%s/clients", $this->getXPath(), $uuid);
    }

    private function getClientXPathByUuid($uuid)
    {
        return sprintf("%s/client[uuid='%s']", $this->getClientsXPath(), $uuid);
    }

    /* /Helpers */

    public function getName()
    {
        return "OpenVPN";
    }

    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
        $this->registerMethod("getList");
        $this->registerMethod("get");
        $this->registerMethod("set");
        $this->registerMethod("delete");
        $this->registerMethod("downloadCertificate");
    }

    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Get configuration object
        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath());

        // Modify result data
        $settingsObject["enable"]           = boolval($object["enable"]);
        $settingsObject["port"]             = intval($object["port"]);
        $settingsObject["protocol"]         = $object["protocol"];
        $settingsObject["compression"]      = boolval($object["compression"]);
        $settingsObject["extra_options"]    = $object["extra_options"];
        $settingsObject["loglevel"]         = intval($object["loglevel"]);
        $settingsObject["vpn_network"]      = $object["vpn_network"];
        $settingsObject["vpn_mask"]         = $object["vpn_mask"];
        $settingsObject["client_to_client"] = boolval($object["client_to_client"]);
        $settingsObject["dns"]              = $object["dns"];
        $settingsObject["dns_domains"]      = $object["dns_domains"];
        $settingsObject["wins"]             = $object["wins"];

        return $settingsObject;
    }

    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "enable"           : { "type" : "boolean" },
                "port"             : { "type" : "integer" },
                "protocol"         : { "type" : "string", "enum" : ["udp", "tcp"] },
                "compression"      : { "type" : "boolean" },
                "extra_options"    : { "type" : "string", "optional" : true},
                "loglevel"         : { "type" : "integer", "enum" : [0, 2, 5, 7] },
                "vpn_network"      : { "type" : "string" },
                "vpn_mask"         : { "type" : "string" },
                "client_to_client" : { "type" : "boolean" },
                "dns"              : { "type" : "string", "optional" : true },
                "dns_domains"      : { "type" : "string", "optional" : true },
                "wins"             : { "type" : "string", "optional" : true }
            }
        }');

        // Prepare configuration data
        $object = array(
            "enable"           => array_boolval($params, "enable"),
            "port"             => intval($params["port"]),
            "protocol"         => $params["protocol"],
            "compression"      => array_boolval($params, "compression"),
            "extra_options"    => $params["extra_options"],
            "loglevel"         => intval($params["loglevel"]),
            "vpn_network"      => $params["vpn_network"],
            "vpn_mask"         => $params["vpn_mask"],
            "client_to_client" => array_boolval($params, "client_to_client"),
            "dns"              => $params["dns"],
            "dns_domains"      => $params["dns_domains"],
            "wins"             => $params["wins"],
        );

        // Get old configuration object
        $oldConfig = $xmlConfig->get($this->getXPath());

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath());

        $object["clients"] = $oldConfig["clients"];

        // Set configuration object
        if (false === $xmlConfig->replace($this->getXPath(), $object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXPath());

        // Notify configuration changes
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(OMV_NOTIFY_MODIFY, $this->getEventMessagePath(), $object);

        return $object;
    }

    public function getList($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : {' . $GLOBALS["OMV_JSONSCHEMA_COUNTFIELD"] . '},
                "sortfield" : {' . $GLOBALS["OMV_JSONSCHEMA_SORTFIELD"] . '},
                "sortdir"   : {' . $GLOBALS["OMV_JSONSCHEMA_SORTDIR"] . '}
            }
        }');

        // Get configuration object
        $xPath = sprintf("%s/client/uuid", $this->getClientsXPath());
        $uuids = $xmlConfig->getList($xPath);

        if (is_null($uuids))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath);

        $clients = array();

        foreach ($uuids as $uuid) {
            try {
                $clients[] = $this->get(array("uuid" => $uuid), $context);
            } catch (Exception $e) {
                throw $e;
            }
        }

        return $this->applyFilter(
            $clients,
            $params["start"],
            $params["count"],
            $params["sortfield"],
            $params["sortdir"]
        );
    }

    public function get($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '}
            }
        }');

        $client = $xmlConfig->get($this->getClientXPathByUuid($params["uuid"]));

        if (is_null($client))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath);

        return $client;
    }

    public function set($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"        : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_UNDEFINED"] . '},
                "user"        : { "type" : "string" },
                "common_name" : { "type" : "string" }
            }
        }');

        $client = array(
            "uuid"        => empty($params["uuid"]) ? OMVUtil::uuid() : $params["uuid"],
            "user"        => $params["user"],
            "common_name" => $params["common_name"],
        );

        // Save configuration
        $success = false;
        $xPath = null;

        if (empty($params["uuid"]))
            $success = $xmlConfig->set($xPath = $this->getClientsXPath(),
                array("client" => $client),
                $client);
        else
            $success = $xmlConfig->replace($xPath = $this->getClientXPathByUuid($params["uuid"]),
                $client);

        if (!$success)
            throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
                $xPath);

        // Generate certificate
        $cmd = sprintf("sudo omv-mkconf %s add %s 2>&1",
            strtolower($this->getName()),
            $client["uuid"]);

        if (0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
                $cmd, implode("\n", $output));

        return $client;
    }

    public function delete($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : {' . $GLOBALS["OMV_JSONSCHEMA_UUID_NONE"] . '}
            }
        }');

        // Revoke certificate and then delete from xml
        // if successful
        $cmd = sprintf("sudo omv-mkconf %s revoke %s 2>&1",
            strtolower($this->getName()),
            $params["uuid"]);

        if (0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
                $cmd, implode("\n", $output));

        $client = $xmlConfig->delete($this->getClientXPathByUuid($params["uuid"]));

        if (is_null($client))
            throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
                $this->getClientXPathByUuid($params["uuid"]));
    }

    public function downloadCertificate($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : {' . $GLOBALS["OMV_JSONSCHEMA_UUID"] . '}
            }
        }');

        $tmpFile = tempnam(sys_get_temp_dir(), "zip");
        $zip = new \ZipArchive();
        $zip->open($tmpFile, \ZIPARCHIVE::CREATE | \ZIPARCHIVE::OVERWRITE);

        $keyDir = "/etc/openvpn/keys";

        $files = array(
            "ca.crt"     => sprintf("%s/ca.crt", $keyDir),
            "client.crt" => sprintf("%s/%s.crt", $keyDir, $params["uuid"]),
            "client.key" => sprintf("%s/%s.key", $keyDir, $params["uuid"])
        );

        foreach ($files as $filename => $file) {
            $zip->addFile($file, $filename);
        }

        $content = file_get_contents($tmpFile);

        return array(
            "contenttype" => "application/zip",
            "filename"    => "openvpn.zip",
            "filepath"    => $tmpFile,
        );
    }

}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceOpenVPN());
