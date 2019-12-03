extends "res://scripts/View.gd"

enum STATE {
	IDLE,
	ERR,
	ORIGIN_SET,
	WAITING_RECEIVER,
	RECEIVER_REFUSED,
	TRACKING_ORIGIN,
	WAITING_SEND,
	TRACKING,
	ARRIVED,
	OPENED,
	DELIVERY_REQUESTED,
	SETTING_CURRENT_LOCATION,
}

enum CLIENT_TYPE {
	NONE,
	SENDER,
	RECEIVER
}

signal state_changed(state)

var current_client = CLIENT_TYPE.NONE
var current_state = STATE.IDLE setget set_state
var origin:int = -1
var destination:int = -1
var last_request_id = -1
var last_request_origin = 0
var qr_id = -1

func _ready():
	DLVR.connect("name_changed", self, "_on_name_changed")
	for h in $city.get_children():
		if h is House:
			h.connect('house_pressed', self, '_on_house_pressed')
	DLVR.client.connect("packet_received", self, "_on_packet_received")
	DLVR.client.connect("server_disconnected", self, "_on_server_disconnected")
	self.current_state = STATE.IDLE

func _on_server_disconnected():
	$err.popup_m("ERRDISC_KS")
	yield($err, "popup_hide")
	DLVR.logout()
	get_tree().change_scene("res://scenes/views/loginView.tscn")

func _on_packet_received(data_r):
	var json_parser = JSON.parse(data_r)
	if json_parser.error == OK:
		var resp = json_parser.result
		if 'path' in resp:
			var data = null
			var id = null
			if resp["path"] == "/delivery/request":
				if current_state == STATE.IDLE:
					last_request_id = resp["message_body"]["id"]
					var orig = resp["message_body"]["origin"]
					var h = get_node("city/node%d"%orig) as House
					if h:
						$pointer_origin.position = h.position
						$pointer_origin.pop(orig)
					$confirm_window.email = resp["message_body"]["sender"]
					self.current_state = STATE.DELIVERY_REQUESTED
				else:
					Utils.print_log("Delivery Request not accept because isnt idle")
					data = {"path": "/delivery/response", "cookie": DLVR.cookie, "id": last_request_id, "accept": false}
					DLVR.client.send_data(JSON.print(data))
			elif resp["path"] == "/delivery/response":
				if current_state == STATE.WAITING_RECEIVER:
					if resp["message_body"]["accept"]:
						last_request_id = resp["message_body"]["id"]
						self.current_state = STATE.TRACKING_ORIGIN
						self.current_client = CLIENT_TYPE.SENDER
						#
						var dest = resp["message_body"]["destination"]
						var h = (get_node("city/node%d"%dest) as House)
						if h:
							$pointer_destination.position = h.position
							$pointer_destination.pop(dest)
					else:
						self.current_state = STATE.RECEIVER_REFUSED
			elif resp["path"] == "/robot/update":
				if ("message_body" in resp) and ("position" in resp["message_body"]):			
					var pid = resp["message_body"]["position"]
					if current_state == STATE.TRACKING:
						var n = get_node("city/node%d"%pid)
						if n:
							$pointer_tracking.position = n.position
							$pointer_tracking.pop(pid)
						if (current_client == CLIENT_TYPE.RECEIVER) and \
						   ("state" in resp["message_body"]) and \
						   (resp["message_body"]["state"] == 1) and \
						   (pid == destination):
							data = {"path": "/delivery/qr", "cookie": DLVR.cookie, "id": last_request_id}
							DLVR.client.send_data(JSON.print(data))
							var r = yield(DLVR.client, "packet_received")
							while r is GDScriptFunctionState:
								r = yield(r, "completed")
							json_parser = JSON.parse(r)
							#FIXME: If not recive QR id the app will not work
							if json_parser.error == OK:
								resp = json_parser.result
								if resp["status_code"] == 200:
									qr_id = resp["message_body"]
							self.current_state = STATE.ARRIVED
					elif current_state == STATE.TRACKING_ORIGIN:
						var n = get_node("city/node%d"%pid)
						if n:
							$pointer_tracking.position = n.position
							$pointer_tracking.pop(pid)
						if (pid == origin) and \
						(current_client == CLIENT_TYPE.SENDER) and \
						(resp["message_body"]["state"] == 1):
							Utils.print_log("Robot arrive at origin")
							self.current_state = STATE.WAITING_SEND
				elif (current_state == STATE.ARRIVED) and \
					 ("path" in resp) and \
					 (resp["path"] == "/robot/update") and \
					 ("qr" in resp["message_body"]):
					self.current_state = STATE.OPENED
			elif resp["path"] == "/delivery/finish":
				__finish()

func _on_house_pressed(id):
	if current_state in [STATE.IDLE, STATE.ORIGIN_SET]:
		origin = id
		var h = (get_node("city/node%d"%id) as House)
		if h:
			$pointer_origin.position = h.get_pos()
			$pointer_origin.pop(id)
		self.current_state = STATE.ORIGIN_SET
	elif current_state in [STATE.SETTING_CURRENT_LOCATION]:
		destination = id
		var h = (get_node("city/node%d"%id) as House)
		if h:
			$pointer_destination.position = h.get_pos()
			$pointer_destination.pop(id)

func _on_name_changed(first_name, last_name):
	$leftBar/header/nameLabel.text = "%s %s" % [first_name, last_name]

func _on_sideMenuButton_pressed():
	$leftBar.show()

func _on_exit_pressed():
	# warning-ignore: return_value_discarded
	if DLVR.logout():		# FIXME: successful request
		get_tree().change_scene("res://scenes/views/loginView.tscn")

func _on_debug_open_pressed():
	DLVR._debug_open_box()

func _on_debug_close_pressed():
	DLVR._debug_close_box()

func _on_cancel_button_pressed():
	if current_state == STATE.ORIGIN_SET:
		yield($pointer_origin.unpop(), "completed")
		$pointer_origin.position = $rest_area.position
	elif current_state == STATE.SETTING_CURRENT_LOCATION:
		yield($pointer_destination.unpop(), "completed")
		$pointer_destination.position = $rest_area.position
		var data = {"path": "/delivery/response", "cookie": DLVR.cookie, "accept": false}
		DLVR.client.send_data(JSON.print(data))
	self.current_state = STATE.IDLE

func _on_confirm_button_pressed():
	var data = null
	var d_email = $delivery_menu/email.text.strip_edges()
	if d_email == DLVR.user_email:
		show_error("AUTOSEND_KS")
	elif current_state == STATE.ORIGIN_SET:
		if $delivery_menu/email.valid:
			data = {
				"path": "/delivery/request",
				"cookie": DLVR.cookie,
				"origin": $pointer_origin.point_id,
				"receiver": d_email,
			}
			DLVR.client.send_data(JSON.print(data))
			self.current_state = STATE.WAITING_RECEIVER
			DLVR.client.start_receive_timer()
			var r = yield(DLVR.client, "packet_received_or_timeout")
			while r is GDScriptFunctionState:
				r = yield(r, "completed")
			if r[0] != Client.OK:
				show_error("ERRTOUT_KS")
				return
			var jsonparser = JSON.parse(r[1])
			if jsonparser.error == OK:
				var resp = jsonparser.result
				if (resp["status_code"] == 200) and \
				   (resp["message_body"] is String) and \
				   (resp["message_body"] == "true"):
					self.current_state = STATE.WAITING_RECEIVER
				elif (resp["status_code"] == 200) and \
				   (resp["message_body"] is String) and \
				   (resp["message_body"] == "false"):
					show_error("ERRMAIL_KS")
				else:
					show_error("ERRUNKW_KS")
	elif current_state == STATE.SETTING_CURRENT_LOCATION:
		data = {
			"path": "/delivery/response",
			"cookie": DLVR.cookie,
			"accept": true,
			"id": last_request_id,
			"destination": $pointer_destination.point_id
		}
		DLVR.client.send_data(JSON.print(data))
		DLVR.client.start_receive_timer()
		var r = yield(DLVR.client, "packet_received_or_timeout")
		while r is GDScriptFunctionState:
			r = yield(r, "completed")
		if r[0] != Client.OK:
			return
		var jsonparser = JSON.parse(r[1])
		if jsonparser.error == OK:
			var resp = jsonparser.result
			if (resp["status_code"] == 200):
				self.current_state = STATE.TRACKING
				self.current_client = CLIENT_TYPE.RECEIVER
			else:
				show_error("ERRTOUT_KS")
				self.current_state = STATE.IDLE

func _on_send_button_pressed():
	if (current_state == STATE.WAITING_SEND) and \
	   (current_client == CLIENT_TYPE.SENDER):
		var data = {"path": "/delivery/send", "cookie": DLVR.cookie, "id": last_request_id}
		DLVR.client.send_data(JSON.print(data))
		DLVR.client.start_receive_timer()
		var r = yield(DLVR.client, "packet_received_or_timeout")
		while r is GDScriptFunctionState:
			r = yield(r, "completed")
		if r[0] != Client.OK:
			show_error("ERRTOUT_KS")
			return
		var jsonparser = JSON.parse(r[1])
		if jsonparser.error == OK:
			var resp = jsonparser.result
			if (resp["status_code"] == 200):
				self.current_state = STATE.TRACKING
			else:
				# show_error("ERRSERV_KS")
				Utils.print_log("<Server> %s" % resp['reason_message'])
		#self.current_state = STATE.TRACKING

func set_state(val):
	current_state = val
	Utils.print_log("STATE {%s}" % str(val))
	if val == STATE.IDLE:
		$pointer_origin.unpop()
		$pointer_destination.unpop()
		$pointer_tracking.unpop()
		#
		$pointer_origin.position = $rest_area.position
		$pointer_destination.position =  $rest_area.position
		$pointer_tracking.position =  $rest_area.position
	emit_signal("state_changed", val)

func _on_finish_button_pressed():
	if current_state == STATE.OPENED:
		var data = {"path": "/delivery/finish", "cookie": DLVR.cookie, "id": last_request_id}
		DLVR.client.send_data(JSON.print(data))
		#
		__finish()


func _on_artags_debug_force_finish():
	var data = {"path": "/delivery/finish", "cookie": DLVR.cookie, "id": last_request_id}
	DLVR.client.send_data(JSON.print(data))
	__finish()

func __finish():
	self.current_client = CLIENT_TYPE.NONE
	self.current_state = STATE.IDLE
	self.origin = -1
	self.destination = -1
	self.last_request_id = -1
	self.qr_id = -1
	#
	$pointer_origin.unpop()
	$pointer_destination.unpop()
	$pointer_tracking.unpop()
	#
	$pointer_origin.position = $rest_area.position
	$pointer_destination.position =  $rest_area.position
	$pointer_tracking.position =  $rest_area.position

func show_error(m:String = "ERRUNKW_KS"):
	self.current_state = STATE.ERR
	$err.popup_m(m)

func _on_err_popup_hide():
	self.current_state = STATE.IDLE
