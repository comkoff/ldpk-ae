	begin objects
		add node checkerboard "c"
		add node ld_3de4_anamorphic_rescaled_degree_6 "ld"
		add node aux_lines "aux"
		add node premultiply "pre"
		add node merge_over_premultiplied "merge"
		add node outfile "out"
	end
	begin relations
		in node "out"
			input 0 merge
		end
		in node "merge"
			input 0 pre
			input 1 ld
		end
		in node "pre"
			input 0 aux
		end
		in node "ld"
			input 0 c
		end
	end
	begin attributes
		in node "c"
			data_window -200 -100 0 2200 1100 1 end
			display_window -200 -100 0 2200 1100 1 end
			colormodel rgb_ end
			field_size 50 50 1 end
		end
		in node "aux"
			data_window [get_data_window_a c:node] [get_data_window_b c:node] end
			display_window [get_display_window_a c:node] [get_display_window_b c:node] end
			premultiplied ext_bool_true end
			styles
				key "data_and_display" end
				value
					color_a 1.0 0.5 0.0 1 end
					color_b 0.5 1.0 0.0 1 end
					stipple "bbbbbbbbaaaaaaaa" end
					width 3 end
				end
			end # styles
                        lines
			end
			crosshairs
			end
			boxes
				key 1 end
				value
					style "data_and_display" end
					position_a 0.0 0.0 end
					position_b 1.0 1.0 end
				end
                        end # boxes
		end
		in node "ld"
			w_fb_cm 2.0 end
			h_fb_cm 1.0 end
			r_pa 1.0 end
			fl_cm 3.0 end
# Mind this! Since data and display are the same, we explicitly
# set the field of view to the broadcast size and position.
			xa_fov_unit 0.08333333333 end
			ya_fov_unit 0.08333333333 end
			xb_fov_unit 0.91666666666 end
			yb_fov_unit 0.91666666666 end
			ld_mode ld_apply end
			cx02_degree_2 0.1 end
			cy02_degree_2 0.2 end
			cx66_degree_6 0.05 end
			cy66_degree_6 -0.1 end
			lens_rotation 2.0 end
			squeeze_x 0.95 end
			squeeze_y 1.05 end
			rescale 2.0 end
			interpol_mode interpol_cubic_mitchell_netravali end
		end
		in node "out"
			filepath "/server/devel/sdv/privat/uwe/source/ldpk/test/flame/openexr/img_b.exr" end
			numeric_representation numrep_floating_point end
			precision 16 end
			format "OPENEXR" end
		end
	end
end
