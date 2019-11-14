extends Node

const SERVER_URL := "wss://ec2-18-229-140-84.sa-east-1.compute.amazonaws.com:443/"
const CLIENT_SCENE = preload("res://scenes/Client.tscn")

signal name_changed(first_name, last_name)

var debug := true setget set_debug

var user_email:String = "dlvr@dlvr.com"
var user_first_name:String = "DL"
var user_last_name:String = "VR"
var cookie:String = "C0081E"
var logged:bool = false 

onready var client:Client = CLIENT_SCENE.instance()


func _ready():
	set_debug(true)
	client.received_timeout = 2.0
	client.connection_timeout = 3.0
	# client.connect("server_disconnected", self, "_server_disconnected")
	add_child(client)


#func _server_disconnected():
#	if logout():
#		get_tree().change_scene("res://scenes/views/loginView.tscn")

func _exit_tree():
	logout()


# FIXME: Error and not registered email is the same
func check_email(email:String):
	var error = client.connect_to_url(DLVR.SERVER_URL + 'user/check')
	if (error == OK) and (yield(client, "connected_or_timeout") == Client.OK):
		client.send_data('{"email":"%s"}' % email)
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			var data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if data['status_code'] == 200:
					if data['message_body'] == 'true':
						client.disconnect_from_host()
						return true
					elif data['message_body'] == 'false':
						client.disconnect_from_host()
						return false
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))
	else:
		Utils.print_log("%s: Error opening connection" % str(self))
	client.disconnect_from_host()
	return false


func signup(email:String, first_name:String, last_name:String, passw:String):
	var data = {
		"email": email,
		"first_name": first_name,
		"last_name": last_name,
		"password": passw
	}
	var error = client.connect_to_url(DLVR.SERVER_URL + 'user/signup')
	if (error == OK) and (yield(client, "connected_or_timeout") == Client.OK):
		client.send_data(JSON.print(data))
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if data['status_code'] == 200:
					client.disconnect_from_host()
					return true
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))
	else:
		Utils.print_log("%s: Error opening connection" % str(self))
	client.disconnect_from_host()
	return false


func verify_code(code, email):
	var data = {
		"user": email,
		"number": code
	}
	var error = client.connect_to_url(DLVR.SERVER_URL + 'code/verify')
	if (error == OK) and (yield(client, "connected_or_timeout") == Client.OK):
		client.send_data(JSON.print(data))
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if (data['status_code'] == 200) and (data['message_body'] == 'true'):
					client.disconnect_from_host()
					return true
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))
	else:
		Utils.print_log("%s: Error opening connection" % str(self))
	client.disconnect_from_host()
	return false


func login(email:String, passw:String):
	if not logged:
		var data = {
			"email": email,
			"password": passw
		}
		var error = client.connect_to_url(DLVR.SERVER_URL + 'user/signin')
		if (error == OK) and (yield(client, "connected_or_timeout") == Client.OK):
			client.send_data(JSON.print(data))
			client.start_receive_timer()
			var res = yield(client, "packet_received_or_timeout")
			if res[0] == Client.OK:
				data = JSON.parse(res[1])
				if data.error == OK:
					data = data.result
					if (data['status_code'] == 200) and (data['message_body'] == 'true'):
						logged = true
						cookie = data['set_cookie']
						user_email = email
						__get_user_profile()
						return true
					else:
						Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
				else:
					Utils.print_log("%s: Error %s" % [str(self), data.error_string])
			else:
				Utils.print_log("%s: Error receive timeout" % str(self))
		else:
			Utils.print_log("%s: Error opening connection" % str(self))
		return false
	return true


func __get_user_profile():
	if logged:
		var data = {
			'path': '/user/profile',
			'cookie': cookie,
		}
		client.send_data(JSON.print(data))
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if (data['status_code'] == 200) and (typeof(data['message_body']) != TYPE_STRING):
					user_first_name = data["message_body"]["first_name"]
					user_last_name = data["message_body"]["last_name"]
					emit_signal("name_changed", user_first_name, user_last_name)
				else:
					Utils.print_log("%s: Error %s" % [str(self), 'No user data'])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))

func set_first_name(val:String):
	user_first_name = val.strip_edges()


func set_last_name(val:String):
	user_last_name = val.strip_edges()


func logout():
	if logged:
		var data = {
			'path': '/user/signout',
			'cookie': cookie
		}
		client.send_data(JSON.print(data))
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if (data['status_code'] == 200) and (data['message_body'] == 'true'):
					logged = false
					user_first_name = "DL"
					user_last_name = "VR"
					cookie = ''
					client.disconnect_from_host()
					return true 					# FIXME: successful request
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))
		return false


func set_debug(val):
	debug = val
	Utils.debug = val


func _debug_open_box():
	if debug and logged:
		var data = {
			'path': '/debug/open',
		}
		client.send_data(JSON.print(data))
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if (data['status_code'] == 200) and (data['message_body'] == 'true'):
					pass
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))


func _debug_close_box():
	if debug and logged:
		var data = {
			'path': '/debug/close',
		}
		client.send_data(JSON.print(data))
		client.start_receive_timer()
		var res = yield(client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if (data['status_code'] == 200) and (data['message_body'] == 'true'):
					pass
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))
