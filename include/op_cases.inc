OP_CASE(op_pop);
OP_CASE(op_dup);
OP_CASE(op_constant);
OP_CASE(op_constant_long);

OP_CASE(op_value);
OP_CASE(op_none);
OP_CASE(op_true);
OP_CASE(op_false);

OP_CASE(op_not);
OP_CASE(op_negate);
OP_CASE(op_equal);
OP_CASE(op_not_equal);
OP_CASE(op_less);
OP_CASE(op_less_equal);
OP_CASE(op_greater);
OP_CASE(op_greater_equal);
OP_CASE(op_add);
OP_CASE(op_subtract);
OP_CASE(op_multiply);
OP_CASE(op_divide);

OP_CASE(op_bw_and);
OP_CASE(op_bw_or);
OP_CASE(op_bw_xor);
OP_CASE(op_bw_sl);
OP_CASE(op_bw_sr);
OP_CASE(op_bw_not);

OP_CASE(op_define_global);
OP_CASE(op_get_global);
OP_CASE(op_set_global);
OP_CASE(op_get_local);
OP_CASE(op_set_local);
OP_CASE(op_get_upvalue);
OP_CASE(op_set_upvalue);

OP_CASE(op_get_property);
OP_CASE(op_set_property);
OP_CASE(op_get_super);
OP_CASE(op_get_layer_property);
OP_CASE(op_get_type);

OP_CASE(op_jump_if_false);
OP_CASE(op_jump_if_neq);
OP_CASE(op_jump);
OP_CASE(op_loop);
OP_CASE(op_call);

OP_CASE(op_closure);
OP_CASE(op_close_upvalue);

OP_CASE(op_print);
OP_CASE(op_return);
OP_CASE(op_break);
OP_CASE(op_continue);
OP_CASE(op_match);

OP_CASE(op_class);
OP_CASE(op_method);
OP_CASE(op_inherit);
OP_CASE(op_invoke);
OP_CASE(op_super_invoke);
OP_CASE(op_struct);
OP_CASE(op_member);
OP_CASE(op_struct_inherit);

OP_CASE(op_enum);
OP_CASE(op_enum_define_member);
OP_CASE(op_enum_get_member);
OP_CASE(op_enum_member_bind);
OP_CASE(op_enum_member_match);

OP_CASE(op_layer_property_call);
OP_CASE(op_vector_new);
OP_CASE(op_vector_set);
OP_CASE(op_vector_get);
