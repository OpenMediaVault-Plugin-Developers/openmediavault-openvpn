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
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/form/field/UserComboBox.js")

Ext.define('OMV.module.admin.service.openvpn.ClientCertificate', {
    extend: 'OMV.workspace.window.Form',
    requires: [
        'OMV.workspace.window.plugin.ConfigObject',
        'OMV.form.field.UserComboBox'
    ],

    plugins: [{
        ptype: 'configobject'
    }],

    rpcService: 'OpenVpn',
    rpcSetMethod: 'set',

    hideResetButton: true,
    uuid: null,

    getFormItems: function() {
        return [{
            xtype: 'fieldset',
            title: _('General'),
            items: [{
                xtype: 'usercombo',
                name: 'associated_user',
                fieldLabel: _('User'),
                userType: 'normal',
                editable: false,
                allowNone: true
            }, {
                xtype: 'textfield',
                name: 'common_name',
                fieldLabel: _('Common name'),
                vtype: 'domainname',
                allowBlank: false
            }]
        }];
    }
});
