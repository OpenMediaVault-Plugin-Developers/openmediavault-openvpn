#!/bin/sh
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2009-2013 Volker Theile
# @copyright Copyright (c) 2013-2019 OpenMediaVault Plugin Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

set -e

. /usr/share/openmediavault/scripts/helper-functions

# EasyRSA version
version="3.0.6"

SERVICE_XPATH_NAME="openvpn"
SERVICE_XPATH="/config/services/${SERVICE_XPATH_NAME}"

if ! omv_config_exists "${SERVICE_XPATH}"; then
    omv_config_add_node "/config/services" "${SERVICE_XPATH_NAME}"
    omv_config_add_key "${SERVICE_XPATH}" "enable" "0"

    omv_config_add_key "${SERVICE_XPATH}" "port" "1194"
    omv_config_add_key "${SERVICE_XPATH}" "protocol" "udp"
    omv_config_add_key "${SERVICE_XPATH}" "compression" "1"
    omv_config_add_key "${SERVICE_XPATH}" "pam_authentication" "0"
    omv_config_add_key "${SERVICE_XPATH}" "extra_options" ""
    omv_config_add_key "${SERVICE_XPATH}" "loglevel" "2"

    omv_config_add_key "${SERVICE_XPATH}" "vpn_network" "10.8.0.0"
    omv_config_add_key "${SERVICE_XPATH}" "vpn_mask" "255.255.255.0"
    omv_config_add_key "${SERVICE_XPATH}" "gateway_interface" ""
    omv_config_add_key "${SERVICE_XPATH}" "default_gateway" "1"
    omv_config_add_key "${SERVICE_XPATH}" "client_to_client" "0"

    omv_config_add_key "${SERVICE_XPATH}" "dns" ""
    omv_config_add_key "${SERVICE_XPATH}" "dns_domains" ""
    omv_config_add_key "${SERVICE_XPATH}" "wins" ""
    omv_config_add_key "${SERVICE_XPATH}" "public_address" ""
    omv_config_add_node "${SERVICE_XPATH}" "clients" ""
fi

if ! omv_group_id_exists openvpn; then
    addgroup --quiet --system openvpn
fi

if [ ! -f "/opt/EasyRSA-$version/easyrsa" ];then 
    wget https://github.com/OpenVPN/easy-rsa/releases/download/v${version}/EasyRSA-unix-v${version}.tgz -P /opt/
    tar xf /opt/EasyRSA-unix-v${version}.tgz -C /opt
    rm -rf /opt/EasyRSA-unix-v${version}.tgz
fi 

exit 0
