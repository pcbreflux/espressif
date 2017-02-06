# Copyright (c) 2017 pcbreflux. All Rights Reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.#
#

# @file websocket_uuid.py
#
# @brief Generate UUID for websocket connection.
#
# @details Generate UUID in base64 format for websocket connection.
#
# @note include mqtt_uuid.h to your project.
#
import uuid
import base64
import sys

websocket_uuid = str(uuid.uuid4()) # Generate a random UUID
websocket_uuid = base64.b64encode(websocket_uuid)
try:
	f = open(sys.argv[1]+'/mqtt_uuid.h', 'wb')
	f.write('#include <stdio.h>\n\n')
	f.write('#define MBEDTLS_WEBSOCKET_UUID \"')
	f.write(websocket_uuid)
	f.write('\"\n')
	f.close()
except:
	print "error open mqtt_uuid.h"


