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
	change_subview($signinSubview.id)

func _on_data_entered(email:String, first_name:String, surname:String, passw:String):
	$passwSubview.initial()
	$passwSubview.set_email(email)
	change_subview($passwSubview.id)

func _on_passw_entered(email:String, passw:String):
	$emailSubview.initial()
	change_subview($emailSubview.id)
	get_tree().change_scene("res://scenes/views/mapView.tscn")

