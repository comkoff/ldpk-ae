	begin objects
		add node infile "myinfile"
		add node outfile "myoutfile"
	end
	begin relations
		in node "myoutfile"
			input 0 myinfile
		end
	end
	begin attributes
		in node "myinfile"
			filepath [getopt "in"] end
		end
		in node "myoutfile"
			filepath [getopt "out"] end
			format "PNG" end
			numeric_representation numrep_unsigned_integer end
			precision 8 end
		end
	end
end
