tool
extends Control

class_name ARTags

export(int, 0, 1) var id:int = 0 setget set_id

func _ready():
	pass

func _draw():
	$tag.texture = load('res://sprites/artags/artag%d.png' % id)

func set_id(val):
	if val > 1:
		id = 1
	elif val < 0:
		id = 0
	else:
		id = val
	update()