extends "res://scripts/View.gd"

func _ready():
	$leftBar/header/nameLabel.text = "%s %s" % [DLVR.user_first_name, DLVR.user_last_name]


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
