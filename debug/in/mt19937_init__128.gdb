set width 0
set height 0
set verbose off

# Setting breaks
break _mt19937_simd.s:71
break _mt19937_simd.s:76
break _mt19937_simd.s:79
break _mt19937_simd.s:100
break _mt19937_simd.s:107

# Start
run

# 71
step
print /u $zmm0.v16_int32
print /u $zmm2.v16_int32
print /u $zmm3.v16_int32

# 76
continue
step
print /u $zmm4.v16_int32

# 79
step
print /u $zmm0.v16_int32

# 100
continue
step
print /u $zmm0.v16_int32

# 107
continue
step
print /u $zmm0.v16_int32

set $i = 1
while $i < 1000
	# 76
	continue
	step
	print /u $zmm4.v16_int32
	
	# 79
	step
	print /u $zmm0.v16_int32
	
	# 100
	continue
	step
	print /u $zmm0.v16_int32
	
	# 107
	continue
	step
	print /u $zmm0.v16_int32

	set $i = $i + 1
end

# End
echo \n \n
quit

