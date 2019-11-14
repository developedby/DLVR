extends "res://scripts/View.gd"

onready var subviews = [$emailSubview, $signinSubview, $passwSubview, $codeSubview]
onready var _client = DLVR.client
var current_subview := 0


func _ready():
	change_subview($emailSubview.id)


func change_subview(vid:int):
	for sv in subviews:
		sv.visible = (sv.id == vid)
		if sv.id == current_subview:
			sv.initial()
	current_subview = vid


func _on_email_entered(email:String):
	var nextView
	var r = yield(DLVR.check_email(email), 'completed')
	if r:
		nextView = $passwSubview
	else:
		nextView = $signinSubview
	nextView.initial()
	nextView.set_email(email)
	change_subview(nextView.id)


func _on_data_entered(email:String, first_name:String, last_name:String, passw:String):
	var r = yield(DLVR.signup(email, first_name, last_name, passw), 'completed')
	if r:
		$codeSubview.initial()
		$codeSubview.set_data(email)
		change_subview($codeSubview.id)


func _on_passw_entered(email:String, passw:String):
	var r = DLVR.login(email, passw)
	if typeof(r) != TYPE_BOOL:
		r = yield(r, 'completed') 
	if r:
		$passwSubview.initial()
		get_tree().change_scene("res://scenes/views/mapView.tscn")
	else:
		$passwSubview/passwField.set_invalid()


func _on_codeSubview_code_entered(code, email):
	var r = yield(DLVR.verify_code(code, email), 'completed') 
	if r:
		$passwSubview.initial()
		$passwSubview.set_email(email)
		change_subview($passwSubview.id)

