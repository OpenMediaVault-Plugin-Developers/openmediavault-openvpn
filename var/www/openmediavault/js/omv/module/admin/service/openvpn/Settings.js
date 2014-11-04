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

// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")

Ext.define("OMV.module.admin.service.openvpn.Settings", {
    extend : "OMV.workspace.form.Panel",

    rpcService   : "OpenVpn",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    getFormItems : function() {
        var me = this;

        return [{
            xtype    : "fieldset",
            title    : _("General settings"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : false
            },{
                xtype         : "numberfield",
                name          : "port",
                fieldLabel    : _("Port"),
                vtype         : "port",
                minValue      : 0,
                maxValue      : 65535,
                allowDecimals : false,
                allowNegative : false,
                value         : 1194
            },{
                xtype      : "combo",
                name       : "protocol",
                fieldLabel : _("Protocol"),
                queryMode  : "local",
                store      : Ext.create("Ext.data.ArrayStore", {
                    fields : [
                        "value",
                        "text"
                    ],
                    data: [
                        [ "udp", "UDP" ],
                        [ "tcp", "TCP" ]
                    ]
                }),
                displayField  : "text",
                valueField    : "value",
                allowBlank    : false,
                editable      : false,
                triggerAction : "all",
                value         : "udp"
            },{
                xtype      : "checkbox",
                name       : "compression",
                fieldLabel : _("Use compression"),
                checked    : true
            },{
                xtype      : "checkbox",
                name       : "pam_authentication",
                fieldLabel : _("PAM authentication"),
                checked    : false,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Authenticate with server using username/password (client certificate and key are still required).")
                }]
            },{
                xtype      : "textarea",
                name       : "extra_options",
                fieldLabel : _("Extra options"),
                allowBlank : true
            },{
                xtype      : "combo",
                name       : "loglevel",
                fieldLabel : _("Logging level"),
                queryMode  : "local",
                store      : Ext.create("Ext.data.ArrayStore", {
                    fields : [
                        "value",
                        "text"
                    ],
                    data   : [
                        [ 0, _("No output except fatal errors") ],
                        [ 2, _("Normal usage output") ],
                        [ 5, _("Log each packet") ],
                        [ 7, _("Debug") ]
                    ]
                }),
                displayField  : "text",
                valueField    : "value",
                allowBlank    : false,
                editable      : false,
                triggerAction : "all",
                value         : 2
            }]
        },{
            xtype    : "fieldset",
            title    : _("VPN network"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "textfield",
                name       : "vpn_network",
                fieldLabel : _("Address"),
                vtype      : "IPv4",
                allowBlank : false,
                value      : "10.8.0.0"
            },{
                xtype      : "textfield",
                name       : "vpn_mask",
                fieldLabel : _("Mask"),
                vtype      : "IPv4",
                allowBlank : false,
                value      : "255.255.255.0"
            },{
                xtype      : "checkbox",
                name       : "default_gateway",
                fieldLabel : _("Default gateway"),
                checked    : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("If enabled, this directive will configure all clients to redirect their default network gateway through the VPN. If disabled, a static route to the private subnet is configured on all clients.")
                }]
            },{
                xtype      : "checkbox",
                name       : "client_to_client",
                fieldLabel : _("Client to client"),
                checked    : false,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Allow client to client communication")
                }]
            }]
        },{
            xtype    : "fieldset",
            title    : _("DHCP options"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "textfield",
                name       : "dns",
                fieldLabel : _("DNS server(s)"),
                vtype      : "IPv4List",
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Separate multiple entries with commas")
                }]
            },{
                xtype      : "textfield",
                name       : "dns_domains",
                fieldLabel : _("DNS search domains(s)"),
                vtype      : "domainnameIPList",
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Separate multiple entries with commas")
                }]
            },{
                xtype      : "textfield",
                name       : "wins",
                fieldLabel : _("WINS server(s)"),
                vtype      : "IPv4List",
                allowBlank : true,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("Separate multiple entries with commas")
                }]
            }]
        },{
            xtype    : "fieldset",
            title    : _("Public"),
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "textfield",
                name       : "public_address",
                fieldLabel : _("Public address"),
                allowBlank : false,
                plugins    : [{
                    ptype : "fieldinfo",
                    text  : _("This is the address which external clients can connect to your VPN with. This is automatically used in the generated configuration.")
                }]
            }]
        }];
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/openvpn",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.admin.service.openvpn.Settings"
});
