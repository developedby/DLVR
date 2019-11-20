tool
extends Node2D


export(int,"origin","destination","tracking") var pointer_type:int = 0 setget set_pointer


func _ready():
	pass

func _draw():
	if pointer_type == 0:
		$sprite.texture = load("res://sprites/pin_red.png")
	elif pointer_type == 1:
		$sprite.texture = load("res://sprites/pin_blue.png")
	else:
		$sprite.texture = load("res://sprites/pin_black.png")

func set_pointer(val):
	pointer_type = val
	update()

func pop():
	$anim.play("pop")

func unpop():
	$anim.play("unpop")
	yield($anim, "animation_finished")
