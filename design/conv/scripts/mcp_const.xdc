set_multicycle_path -setup -end -from [get_cells -hierarchical -filter { NAME =~  "*din4_buf*" && PRIMITIVE_TYPE =~ REGISTER.*.* }] 2
set_multicycle_path -setup -end -from [get_cells -hierarchical -filter { NAME =~  "*din2_buf*" && PRIMITIVE_TYPE =~ REGISTER.*.* }] 2
set_multicycle_path -setup -end -from [get_cells -hierarchical -filter { NAME =~  "*din6_buf*" && PRIMITIVE_TYPE =~ REGISTER.*.* }] 2
set_multicycle_path -hold -end -from [get_cells -hierarchical -filter { NAME =~  "*din4_buf*" && PRIMITIVE_TYPE =~ REGISTER.*.* }] 1
set_multicycle_path -hold -end -from [get_cells -hierarchical -filter { NAME =~  "*din2_buf*" && PRIMITIVE_TYPE =~ REGISTER.*.* }] 1
set_multicycle_path -hold -end -from [get_cells -hierarchical -filter { NAME =~  "*din6_buf*" && PRIMITIVE_TYPE =~ REGISTER.*.* }] 1
set_multicycle_path -setup -start -from [get_clocks [get_clocks -of_objects [get_pins zcu102_i/clk_wiz_0/inst/plle4_adv_inst/CLKOUT0]]] -to [get_clocks [get_clocks -of_objects [get_pins zcu102_i/clk_wiz_0/inst/BUFGCE_DIV_CLK2_inst/O]]] 2
set_multicycle_path -hold -start -from [get_clocks [get_clocks -of_objects [get_pins zcu102_i/clk_wiz_0/inst/plle4_adv_inst/CLKOUT0]]] -to [get_clocks [get_clocks -of_objects [get_pins zcu102_i/clk_wiz_0/inst/BUFGCE_DIV_CLK2_inst/O]]] 1
