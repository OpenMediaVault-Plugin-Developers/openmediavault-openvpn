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

// require("js/omv/PluginManager.js")
// require("js/omv/module/admin/diagnostic/log/plugin/Plugin.js")

Ext.define("OMV.module.admin.diagnostic.log.plugin.OpenVPN", {
    extend : "OMV.module.admin.diagnostic.log.plugin.Plugin",

    id      : "openvpn",
    text    : "OpenVPN",
    columns : [{
        text      : _("Date & Time"),
        sortable  : true,
        dataIndex : "rownum",
        stateId   : "date",
        renderer  : function(value, metaData, record) {
            return record.get("date");
        }
    },{
        text      : _("Event"),
        sortable  : true,
        dataIndex : "event",
        stateId   : "event",
        flex      : 1
    }],
    rpcParams : {
        id : "openvpn"
    },
    rpcFields : [{
        name : "rownum",
        type : "int"
    },{
        name : "ts",
        type : "int"
    },{
        name : "date",
        type : "string"
    },{
        name : "event",
        type : "string"
    }]
});

OMV.PluginManager.register({
    ptype     : "diagnostic",
    id        : "log",
    className : "OMV.module.admin.diagnostic.log.plugin.OpenVPN"
});
