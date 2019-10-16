extends Control

func _ready():
	visible = false


func show():
	visible = true
	$anim.play("show")
	


func hide():
	$anim.play("hide")
	yield($anim, "animation_finished")
	visible = false

func _on_back_pressed():
	hide()
