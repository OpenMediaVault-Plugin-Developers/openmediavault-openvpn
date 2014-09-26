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

Ext.define("OMV.module.user.service.openvpn.Certificates", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc"
    ],

    hidePagingToolbar : false,
    hideAddButton     : true,
    hideDeleteButton  : true,
    hideEditButton    : true,
    hideRefreshButton : false,
    reloadOnActivate  : true,

    columns : [{
        header    : _("UUID"),
        hidden    : true,
        dataIndex : "uuid"
    },{
        header    : _("Common name"),
        flex      : 1,
        sortable  : true,
        dataIndex : "common_name",
    }],

    store : Ext.create("OMV.data.Store", {
        autoLoad   : true,
        remoteSort : false,
        model      : OMV.data.Model.createImplicit({
            idProperty : "uuid",
            fields     : [
                { name : "uuid" },
                { name : "common_name" }
            ]
        }),
        proxy : {
            type    : "rpc",
            rpcData : {
                "service" : "OpenVpn",
                "method"  : "getList"
            }
        }
    }),

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
            disabled : true,
            selectionConfig : {
                minSelections : 1,
                maxSelections : 1
            }
        }]);

        return items;
    },

    onDownloadCertificateButton : function() {
        var me = this,
            record = me.getSelected();

        OMV.Download.request("OpenVpn", "downloadCertificate", {
            uuid: record.get("uuid")
        });
    }

});

OMV.WorkspaceManager.registerPanel({
    id        : "certificates",
    path      : "/service/openvpn",
    text      : _("Certificates"),
    position  : 10,
    className : "OMV.module.user.service.openvpn.Certificates"
});
