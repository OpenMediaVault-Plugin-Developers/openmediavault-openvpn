# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
# @copyright Copyright (c) 2019 OpenMediaVault Plugin Developers
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

{% set config = salt['omv_conf.get']('conf.service.openvpn') %}

{% if config.enable | to_bool %}

update_openmediavault_openvpn_config:
  cmd.run:
    - name: "/usr/sbin/omv-openvpn"

start_openvpn_service:
  service.running:
    - name: openvpn
    - enable: True
    - watch:
      - file: "/etc/openvpn/server.conf"

{% else %}

stop_openvpn_service:
  service.dead:
    - name: openvpn
    - enable: False

{% endif %}
