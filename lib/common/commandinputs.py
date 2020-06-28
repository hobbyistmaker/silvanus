import logging
from collections import namedtuple

import adsk.core

StringValue = namedtuple('StringValue', 'value input')
ItemsValue = namedtuple('ItemsValue', 'items input')

logger = logging.getLogger(__name__)

_expression_inputs = [
	adsk.core.DistanceValueCommandInput,
	adsk.core.FloatSliderCommandInput,
	adsk.core.FloatSpinnerCommandInput,
	adsk.core.IntegerSliderCommandInput,
	adsk.core.IntegerSpinnerCommandInput,
	adsk.core.ValueCommandInput,
	adsk.core.SliderCommandInput
]

_value_inputs = [
	adsk.core.BoolValueCommandInput,
	adsk.core.StringValueCommandInput
]

_list_inputs = [
	adsk.core.ButtonRowCommandInput,
	adsk.core.DropDownCommandInput,
	adsk.core.RadioButtonGroupCommandInput
]

_selection_inputs = [
	adsk.core.SelectionCommandInput
]


def create_value_dict(value, input_):
	return {
		'value': value,
		'input': input_
	}


def _parse_value_input(input_):
	return input_


def _parse_expression_input(input_):
	return input_


def _parse_single_dropdown_input(input_):
	return input_.selectedItem


def _parse_checkbox_dropdown_input(input_):
	return input_.listItems


def _parse_list_input(input_):
	list_parsers = {
		adsk.core.DropDownStyles.CheckBoxDropDownStyle:    _parse_checkbox_dropdown_input,
		adsk.core.DropDownStyles.LabeledIconDropDownStyle: _parse_single_dropdown_input,
		adsk.core.DropDownStyles.TextListDropDownStyle:    _parse_single_dropdown_input,
	}

	parse_func = list_parsers[input_.dropDownStyle]
	return parse_func(input_)


def _parse_selection_input(input_):
	return [
		input_.selection(s).entity for s in range(0, input_.selectionCount) if s < input_.selectionCount
	]


def _parse_unknown_input(input_):
	return create_value_dict(input_.name, input_)


input_map = ((_selection_inputs, _parse_selection_input), (_expression_inputs, _parse_expression_input),
			 (_value_inputs, _parse_value_input), (_list_inputs, _parse_list_input))
input_parsers = { input_type: parser for input_types, parser in input_map for input_type in input_types }


def parse(input_):
	"""Parse the inputs from Fusion.

	:param input_: a CommandInput provided by Fusion
	:return: a dictionary containing the inputs and values
	"""

	# input_values = { }
	#
	# for input_ in inputs:
	# 	clazz = type(input_)
	# 	if clazz in input_parsers:
	# 		parser = input_parsers[clazz]
	# 		input_values[input_.id] = parser(input_)
	#
	# logger.debug(pformat(input_values))
	# return input_values

	clazz = type(input_)
	if clazz in input_parsers:
		parser = input_parsers[clazz]
		return parser(input_)


class FusionInputParser:

	def __init__(self, inputs):
		self._inputs = inputs

	def __getitem__(self, name):
		input_ = self._inputs.itemById(name)
		logger.debug(f'{name} input is {input_}')
		return parse(input_)
