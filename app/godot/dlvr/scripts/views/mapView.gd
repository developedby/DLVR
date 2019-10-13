extends "res://scripts/View.gd"

func _ready():
	$leftBar/header/nameLabel.text = "%s %s" % [DLVR.user_first_name, DLVR.user_surname]


func _on_sideMenuButton_pressed():
	$leftBar.show()
