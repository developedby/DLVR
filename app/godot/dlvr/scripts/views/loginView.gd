extends "res://scripts/View.gd"

onready var subviews = [$emailSubview, $signinSubview, $passwSubview,]
var _client = Client.new()

func _ready():
	change_subview($emailSubview.id)
	add_child(_client)


func change_subview(vid:int):
	for sv in subviews:
		sv.visible = (sv.id == vid)


func _on_email_entered(email:String):
	$signinSubview.initial()
	$signinSubview.set_email(email)
	Utils.print_log('Connection Start...')
	var error = _client.connect_to_url(DLVR.SERVER_URL+'user/check', PoolStringArray())
	if error == OK:
		yield(get_tree().create_timer(1), "timeout")
		_client.send_data('{"email":"%s"}' % email, _client.last_connected_client)
		yield(get_tree().create_timer(1), "timeout")
		_client.disconnect_from_host()


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
