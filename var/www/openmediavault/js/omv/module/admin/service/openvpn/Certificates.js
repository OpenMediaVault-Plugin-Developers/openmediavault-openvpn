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

Ext.define("OMV.module.admin.service.openvpn.Certificates", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.module.admin.service.openvpn.ClientCertificate"
    ],

    hideEditButton : true,

    columns : [{
        header    : _("UUID"),
        hidden    : true,
        dataIndex : "uuid"
    },{
        header    : _("User"),
        flex      : 1,
        sortable  : true,
        dataIndex : "user"
    },{
        header    : _("Common name"),
        flex      : 1,
        sortable  : true,
        dataIndex : "common_name",
    }],

    initComponent : function() {
        var me = this;

        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoload   : true,
                remoteSort : false,
                model      : OMV.data.Model.createImplicit({
                    idProperty   : "uuid",
                    totalPoperty : "total",
                    fields       : [
                        { name : "uuid" },
                        { name : "user" },
                        { name : "common_name" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        "service" : "OpenVPN",
                        "method"  : "getList"
                    }
                }
            })
        });

        me.doReload();

        var selModel = me.getSelectionModel();
        selModel.on("selectionchange", me.updateButtonState, me);

        me.callParent(arguments);
    },

    getTopToolbarItems : function() {
        var me = this;
        var items = me.callParent(arguments);

        Ext.Array.push(items, [{
            id       : me.getId() + "-dowload-certificate",
            xtype    : "button",
            text     : _("Download certificate"),
            icon     : "images/download.png",
            iconCls  : Ext.baseCSSPrefix + "btn-icon-16x16",
            handler  : Ext.Function.bind(me.onDownloadCertificateButton, me, [ me ]),
            scope    : me,
            disabled : true
        }]);

        return items;
    },

    updateButtonState : function() {
        var me = this;

        var records = me.getSelection();

        var action = records.length === 1 ? "enable" : "disable";
        me.queryById(me.getId() + "-dowload-certificate")[action]();
    },

    onAddButton : function() {
        var me = this;

        Ext.create("OMV.module.admin.service.openvpn.ClientCertificate", {
            title        : _("Add certificate"),
            listeners    : {
                scope  : me,
                submit : function() {
                    me.doReload();
                }
            }
        }).show();
    },

    doDeletion : function(record) {
        var me = this;

        OMV.Rpc.request({
            scope : me,
            callback : me.onDeletion,
            rpcData : {
                service : "OpenVPN",
                method : "delete",
                params : {
                    uuid : record.get("uuid")
                }
            }
        });
    },

    onDownloadCertificateButton : function() {
        var me = this,
            record = me.getSelected();

        OMV.Download.request("OpenVPN", "downloadCertificate", {
            uuid: record.get("uuid")
        });
    }

});

OMV.WorkspaceManager.registerPanel({
    id        : "certificates",
    path      : "/service/openvpn",
    text      : _("Certificates"),
    position  : 20,
    className : "OMV.module.admin.service.openvpn.Certificates"
});
