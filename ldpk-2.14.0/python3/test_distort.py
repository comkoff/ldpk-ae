import ldpk

w_fb_cm = 2.0
h_fb_cm = 1.0
fl_cm = 4.0
x_lco_cm = 0.2
y_lco_cm = -0.1

a = ldpk.anamorphic_stabilizer_degree_6()
a.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
a.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

a.cx02(0.05).cy02(0.04).cx22(0.03).cy22(0.02)
a.cx04(0.01).cy04(0.02).cx24(0.03).cy24(0.04).cx44(0.05).cy44(0.04)
a.cx06(0.03).cy06(0.02).cx26(0.01).cy26(0.02).cx46(0.03).cy46(0.04).cx66(0.05).cy66(0.06)
a.sx(0.95).sy(1.05)
a.phi_mnt(1.5);
a.s_rscl(1.5)
a.stbx(0.1).stby(0.05)
a.prepare()

b = ldpk.anamorphic_standard_degree_4()
b.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
b.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

b.cx02(0.05).cy02(0.04).cx22(0.03).cy22(0.02)
b.cx04(0.01).cy04(0.02).cx24(0.03).cy24(0.04).cx44(0.05).cy44(0.04)
b.sx(0.95).sy(1.05)
b.phi_mnt(1.5);
b.prepare()

c = ldpk.anamorphic_standard_degree_6()
c.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
c.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

c.cx02(0.05).cy02(0.04).cx22(0.03).cy22(0.02)
c.cx04(0.01).cy04(0.02).cx24(0.03).cy24(0.04).cx44(0.05).cy44(0.04)
c.cx06(0.03).cy06(0.02).cx26(0.01).cy26(0.02).cx46(0.03).cy46(0.04).cx66(0.05).cy66(0.06)
c.sx(0.95).sy(1.05)
c.phi_mnt(1.5);
c.prepare()

d = ldpk.anamorphic_rescaled_degree_4()
d.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
d.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

d.cx02(0.05).cy02(0.04).cx22(0.03).cy22(0.02)
d.cx04(0.01).cy04(0.02).cx24(0.03).cy24(0.04).cx44(0.05).cy44(0.04)
d.sx(0.95).sy(1.05)
d.phi_mnt(1.5);
d.s_rscl(1.5)
d.prepare()

e = ldpk.anamorphic_rescaled_degree_6()
e.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
e.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

e.cx02(0.05).cy02(0.04).cx22(0.03).cy22(0.02)
e.cx04(0.01).cy04(0.02).cx24(0.03).cy24(0.04).cx44(0.05).cy44(0.04)
e.cx06(0.03).cy06(0.02).cx26(0.01).cy26(0.02).cx46(0.03).cy46(0.04).cx66(0.05).cy66(0.06)
e.sx(0.95).sy(1.05)
e.phi_mnt(1.5);
e.s_rscl(1.5)
e.prepare()

f = ldpk.radial_standard_degree_4()
f.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
f.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

f.c2(0.05).u2(0.04).v2(0.03)
f.c4(0.05).u4(0.04).v4(0.03)
f.phi_bs(20.0);
f.b_bs(1.03);
f.prepare()

g = ldpk.radial_stabilizer_degree_6()
g.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)
g.x_lco_cm(x_lco_cm).y_lco_cm(y_lco_cm)

g.c2(0.05).u2(0.04).v2(0.03)
g.c4(0.05)
g.c6(0.02)
g.stbx(0.1).stby(0.05)
g.prepare()

points = ((0.01,0.01),(0.99,0.99),(0.01,0.99),(0.99,0.99))
print("anamorphic_stabilizer_degree_6")
for p in points:
	q = a.undistort(p)
	r = a.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))
print("anamorphic_standard_degree_4")
for p in points:
	q = b.undistort(p)
	r = b.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))
print("anamorphic_standard_degree_6")
for p in points:
	q = c.undistort(p)
	r = c.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))
print("anamorphic_rescaled_degree_4")
for p in points:
	q = d.undistort(p)
	r = d.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))
print("anamorphic_rescaled_degree_6")
for p in points:
	q = e.undistort(p)
	r = e.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))
print("radial_standard_degree_4")
for p in points:
	q = f.undistort(p)
	r = f.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))
print("radial_stabilizer_degree_6")
for p in points:
	q = g.undistort(p)
	r = g.distort(q)
	print('{0:.12f}'.format(p[0]),'{0:.12f}'.format(p[1]))
	print('{0:.12f}'.format(r[0]),'{0:.12f}'.format(r[1]))

print("anamorphic_stabilizer_degree_6")
for p in points:
	J1 = a.getJacobianMatrixDQ(p)
	J2 = a.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
print("anamorphic_standard_degree_4")
for p in points:
	J1 = b.getJacobianMatrixDQ(p)
	J2 = b.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
print("anamorphic_standard_degree_6")
for p in points:
	J1 = c.getJacobianMatrixDQ(p)
	J2 = c.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
print("anamorphic_standard_degree_6")
for p in points:
	J1 = d.getJacobianMatrixDQ(p)
	J2 = d.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
print("anamorphic_rescaled_degree_6")
for p in points:
	J1 = e.getJacobianMatrixDQ(p)
	J2 = e.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
print("radial_standard_degree_4")
for p in points:
	J1 = f.getJacobianMatrixDQ(p)
	J2 = f.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
print("radial_stabilizer_degree_6")
for p in points:
	J1 = g.getJacobianMatrixDQ(p)
	J2 = g.getJacobianMatrix(p)
	print('{0:.6f}'.format(J1[0]),'{0:.6f}'.format(J1[1]),'{0:.6f}'.format(J1[2]),'{0:.6f}'.format(J1[3]))
	print('{0:.6f}'.format(J2[0]),'{0:.6f}'.format(J2[1]),'{0:.6f}'.format(J2[2]),'{0:.6f}'.format(J2[3]))
	
