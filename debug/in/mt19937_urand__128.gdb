set width 0
set height 0
set verbose off

# Setting breaks
break _mt19937_simd.s:155
break _mt19937_simd.s:169
break _mt19937_simd.s:171
break _mt19937_simd.s:192
break _mt19937_simd.s:204
break _mt19937_simd.s:210
break _mt19937_simd.s:240

# Start
run

# 155
step
print /u $rbx
print /u $rax

# 169
continue
step
print /u $zmm0.v16_int32
print /u $zmm1.v16_int32

# 171
step
print /u $zmm0.v16_int32

# 192
continue
step
print /u $zmm1.v16_int32

# 204
continue
step
print /u $rax

#210
continue
step
print /u $zmm0.v16_int32

# 240
continue
step
print /u $zmm0.v16_int32

set $i = 1
while $i < 1000
	# 155
	continue
	step
	print /u $rbx
	print /u $rax

	# 169
	continue
	step
	print /u $zmm0.v16_int32
	print /u $zmm1.v16_int32
	
	# 171
	step
	print /u $zmm0.v16_int32
	
	# 192
	continue
	step
	print /u $zmm1.v16_int32
	
	# 204
	continue
	step
	print /u $rax

	# 210
	continue
	step
	print /u $zmm0.v16_int32
	
	# 240
	continue
	step
	print /u $zmm0.v16_int32

	set $i = $i + 1
end

# End
echo \n \n
quit

