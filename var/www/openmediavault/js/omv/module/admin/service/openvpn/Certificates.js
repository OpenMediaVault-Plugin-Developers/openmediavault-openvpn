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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")
// require("js/omv/data/Download.js")
// require("js/omv/module/admin/service/openvpn/ClientCertificate.js")

Ext.define('OMV.module.admin.service.openvpn.Certificates', {
    extend: 'OMV.workspace.grid.Panel',
    requires: [
        'OMV.data.Store',
        'OMV.data.Model',
        'OMV.data.proxy.Rpc',
        'OMV.module.admin.service.openvpn.ClientCertificate'
    ],

    hidePagingToolbar: false,
    hideEditButton: true,
    reloadOnActivate: true,

    columns: [{
        header: _('UUID'),
        hidden: true,
        dataIndex: 'uuid'
    }, {
        header: _('User'),
        flex: 1,
        sortable: true,
        dataIndex: 'associated_user'
    }, {
        header: _('Common name'),
        flex: 1,
        sortable: true,
        dataIndex: 'common_name',
    }],

    store: Ext.create('OMV.data.Store', {
        autoLoad: true,
        remoteSort: false,
        model: OMV.data.Model.createImplicit({
            idProperty: 'uuid',
            fields: [{
                name: 'uuid'
            }, {
                name: 'associated_user'
            }, {
                name: 'common_name'
            }]
        }),
        proxy: {
            type: 'rpc',
            rpcData: {
                'service': 'OpenVpn',
                'method': 'getList'
            }
        }
    }),

    getTopToolbarItems: function() {
        var items = this.callParent(arguments);

        Ext.Array.push(items, [{
            id: this.getId() + '-dowload-certificate',
            xtype: 'button',
            text: _('Download certificate'),
            icon: 'images/download.png',
            iconCls: Ext.baseCSSPrefix + 'btn-icon-16x16',
            handler: Ext.Function.bind(this.onDownloadCertificateButton, this),
            scope: this,
            disabled: true,
            selectionConfig: {
                minSelections: 1,
                maxSelections: 1
            }
        }]);

        return items;
    },

    onAddButton: function() {
        Ext.create('OMV.module.admin.service.openvpn.ClientCertificate', {
            title: _('Add certificate'),
            uuid: OMV.UUID_UNDEFINED,
            listeners: {
                scope: this,
                submit: function() {
                    this.doReload();
                }
            }
        }).show();
    },

    doDeletion: function(record) {
        OMV.Rpc.request({
            scope: this,
            callback: this.onDeletion,
            rpcData: {
                service: 'OpenVpn',
                method: 'delete',
                params: {
                    uuid: record.get('uuid')
                }
            }
        });
    },

    onDownloadCertificateButton: function() {
        var record = this.getSelected();

        OMV.Download.request('OpenVpn', 'downloadCertificate', {
            uuid: record.get('uuid')
        });
    }

});

OMV.WorkspaceManager.registerPanel({
    id: 'certificates',
    path: '/service/openvpn',
    text: _('Certificates'),
    position: 20,
    className: 'OMV.module.admin.service.openvpn.Certificates'
});
