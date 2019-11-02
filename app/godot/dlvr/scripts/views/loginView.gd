extends "res://scripts/View.gd"

onready var subviews = [$emailSubview, $signinSubview, $passwSubview,]


func _ready():
	change_subview($emailSubview.id)

func change_subview(vid:int):
	for sv in subviews:
		sv.visible = (sv.id == vid)

func _on_email_entered(email:String):
	$signinSubview.initial()
	$signinSubview.set_email(email)
	if DLVR.debug:
		print('Connection Start...')
	print("Error " + str(DLVR.ws.connect_to_url(DLVR.SERVER_URL + 'connect/')))
	if DLVR.ws.get_connection_status() == DLVR.ws.CONNECTION_CONNECTING || DLVR.ws.get_connection_status() == DLVR.ws.CONNECTION_CONNECTED:
		DLVR.ws.poll()
		DLVR.ws.get_peer(1).put_var(JSON.print({'email':email}))
		print('send')
		if DLVR.ws.get_peer(1).get_available_packet_count() > 0 :
			print('recived')
			var result = DLVR.ws.get_peer(1).get_var(true)
			print(result)
			if result['status_code'] == 200 and result['message_body'] == 'true':
				change_subview($signinSubview.id)
			else:
				$emailSubview/emailField.valid = false
		DLVR.ws.disconnect_from_host()
	if DLVR.debug:
		print('...Connection End')
	
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

