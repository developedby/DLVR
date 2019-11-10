extends "res://scripts/View.gd"

onready var subviews = [$emailSubview, $signinSubview, $passwSubview,]
onready var _client = $client
var current_subview := 0


func _ready():
	change_subview($emailSubview.id)


func change_subview(vid:int):
	for sv in subviews:
		sv.visible = (sv.id == vid)
	current_subview = vid


func _process(delta):
	set_process(false)


func _on_email_entered(email:String):
	Utils.print_log(email)
	var error = _client.connect_to_url(DLVR.SERVER_URL + 'user/check')
	if (error == OK) and (yield(_client, "connected_or_timeout") == Client.OK):
		_client.send_data('{"email":"%s"}' % email)
		_client.start_receive_timer()
		var res = yield(_client, "packet_received_or_timeout")
		if res[0] == Client.OK:
			var data = JSON.parse(res[1])
			if data.error == OK:
				data = data.result
				if data['status_code'] == 200:
					var nextView
					if data['message_body'] == 'true':
						nextView = $passwSubview
					else:
						nextView = $signinSubview
					nextView.initial()
					nextView.set_email(email)
					change_subview(nextView.id)
				else:
					Utils.print_log("%s: Error %s" % [str(self), data['reason_message']])
			else:
				Utils.print_log("%s: Error %s" % [str(self), data.error_string])
		else:
			Utils.print_log("%s: Error receive timeout" % str(self))
		_client.disconnect_from_host()
	else:
		Utils.print_log("%s: Error opening connection" % str(self))


func _on_data_entered(email:String, first_name:String, last_name:String, passw:String):
	DLVR.set_first_name(first_name)
	DLVR.set_last_name(last_name)
	$passwSubview.initial()
	$passwSubview.set_email(email)
	change_subview($passwSubview.id)


func _on_passw_entered(email:String, passw:String):
	$emailSubview.initial()
	change_subview($emailSubview.id)
	get_tree().change_scene("res://scenes/views/mapView.tscn")
