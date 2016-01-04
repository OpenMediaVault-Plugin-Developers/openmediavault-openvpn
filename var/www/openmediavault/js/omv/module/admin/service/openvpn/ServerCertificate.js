/**
 * Copyright (C) 2015 OpenMediaVault Plugin Developers
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
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/form/field/UserComboBox.js")

Ext.define("OMV.module.admin.service.openvpn.ServerCertificate", {
    extend: "OMV.workspace.window.Form",
    requires: [
        "OMV.workspace.window.plugin.ConfigObject",
        "OMV.form.field.UserComboBox"
    ],

    plugins: [{
        ptype: "configobject"
    }],

    rpcService: "OpenVpn",
	rpcGetMethod: "getServer",
    rpcSetMethod: "setServer",

    hideResetButton: true,
    uuid: null,

    getFormItems: function() {
        return [{
            xtype: "fieldset",
            title: _("Create Server Certificate"),
            items: [{
                xtype: "textfield",
                name: "server_common_name",
                fieldLabel: _("Common name"),
                vtype: "domainname",
                allowBlank: false
			}, {
				xtype: "textfield",
				name: "server_country",
				fieldLabel: _("Country"),
				allowBlank: false
			}, {
				xtype: "textfield",
				name: "server_province",
				fieldLabel: _("Province / State"),
				allowBlank: false
			}, {
				xtype: "textfield",
				name: "server_city",
				fieldLabel: _("City"),
				allowBlank: false
			}, {
				xtype: "textfield",
				name: "server_organization",
				fieldLabel: _("Organization"),
				allowBlank: false
			}, {
				xtype: "textfield",
				name: "server_email",
				fieldLabel: _("E-mail"),
				vtype: "email",
				allowBlank: false
            }]
        }];
    }
});
