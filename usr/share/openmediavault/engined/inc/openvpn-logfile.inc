<?php

/**
 * Copyright (C) 2013-2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Register new logfile type.
OMVLogFile::registerType(
    "openvpn",
    [
        "filename" => "openvpn",
        "filepath" => "/var/log/openvpn.log",
        "regex" => "/^(\w+\s\w+\s+\d+\s+\d+:\d+:\d+\s\d+)\s+(.*)$/",
        "columns" => [
            "ts" => [
                "index" => 1,
                "func" => create_function(
                    "\$v",
                    "return strpdate(\$v, \"D M d H:i:s Y\");"
                )
            ],
            "date" => [
                "index" => 1,
                "func" => create_function(
                    "\$v",
                    "return strftime(\"%c\", strpdate(\$v,\"D M d H:i:s Y\"));"
                )
            ],
            "event" => 2
        ]
    ]
);
