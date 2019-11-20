extends "res://scripts/View.gd"

enum STATE {
	IDLE,
	ORIGIN_SET,
	WAITING_RECEIVER,
	RECEIVER_REFUSED,
	TRACKING,
	DELIVERY_REQUESTED,
	SETTING_CURRENT_LOCATION,
}

signal state_changed(state)

var current_state = STATE.IDLE setget set_state
var origin:int = -1
var destination:int = -1

func _ready():
	DLVR.connect("name_changed", self, "_on_name_changed")
	for h in $city.get_children():
		if h is House:
			h.connect('house_pressed', self, '_on_house_pressed')
	DLVR.client.connect("packet_received", self, "_on_packet_received")

func _on_packet_received(data_r):
	var json_parser = JSON.parse(data_r)
	if json_parser.error == OK:
		var resp = json_parser.result
		print(resp)
		if 'path' in resp:
			var data = null
			var id = null
			if resp["path"] == "/delivery/request":
				if current_state == STATE.IDLE:
					$confirm_window.id = resp["message_body"]["id"]
					self.current_state = STATE.DELIVERY_REQUESTED
				else:
					Utils.print_log("Delivery Request not accept because isnt idle")
					data = {"path": "/delivery/response", "cookie": DLVR.cookie, "accept": false}
					DLVR.client.send_data(JSON.print(data))

func _on_house_pressed(id):
	if current_state in [STATE.IDLE, STATE.ORIGIN_SET]:
		origin = id
		var h = (get_node("city/node%d"%id) as House)
		if h:
			$pointer_origin.position = h.get_pos()
			$pointer_origin.pop()
		self.current_state = STATE.ORIGIN_SET
	elif current_state in [STATE.SETTING_CURRENT_LOCATION]:
		destination = id
		var h = (get_node("city/node%d"%id) as House)
		if h:
			$pointer_destination.position = h.get_pos()
			$pointer_destination.pop()

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
	if current_state == STATE.ORIGIN_SET:
		# Check email
		# Send Delivery Request
		pass
	elif current_state == STATE.SETTING_CURRENT_LOCATION:
		self.current_state = STATE.TRACKING
		#var data = {"path": "/delivery/response", "cookie": DLVR.cookie, "accept": false}
		#DLVR.client.send_data(JSON.print(data))

func set_state(val):
	current_state = val
	emit_signal("state_changed", val)