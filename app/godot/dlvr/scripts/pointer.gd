tool
extends Node2D


export(int,"origin","destination","tracking") var pointer_type:int = 0 setget set_pointer
var point_id:int = -1

func _ready():
	pass

func _draw():
	if pointer_type == 0:
		$sprite.texture = load("res://sprites/pin_red.png")
	elif pointer_type == 1:
		$sprite.texture = load("res://sprites/pin_green.png")
	else:
		$sprite.texture = load("res://sprites/pin_black.png")

func set_pointer(val):
	pointer_type = val
	update()

func pop(val=null):
	$anim.play("pop")
	if (val as int) != null:
		point_id = (val as int)

func unpop():
	point_id = -1
	$anim.play("unpop")
	yield($anim, "animation_finished")
