// data.c

void ortho3(gs_vec3* left, gs_vec3* up, gs_vec3 v) {
	*left = (v.z*v.z) < (v.x*v.x) ? gs_v3(v.y,-v.x,0) : gs_v3(0,-v.z,v.y);
	*up = gs_vec3_cross(*left, v);
}

gs_poly_t gs_pyramid_poly(gs_vec3 from, gs_vec3 to, float size) {
    /* calculate axis */
    gs_vec3 up, right, forward = gs_vec3_norm( gs_vec3_sub(to, from) );
    ortho3(&right, &up, forward);

    /* calculate extend */
    gs_vec3 xext = gs_vec3_scale(right, size);
    gs_vec3 yext = gs_vec3_scale(up, size);
    gs_vec3 nxext = gs_vec3_scale(right, -size);
    gs_vec3 nyext = gs_vec3_scale(up, -size);

    /* calculate base vertices */
    gs_poly_t p = {0};
    p.verts = gs_malloc(sizeof(*p.verts) * (5+1)); p.cnt = 5; /*+1 for diamond case*/ // array_resize(p.verts, 5+1); p.cnt = 5;
    p.verts[0] = gs_vec3_add(gs_vec3_add(from, xext), yext); /*a*/
    p.verts[1] = gs_vec3_add(gs_vec3_add(from, xext), nyext); /*b*/
    p.verts[2] = gs_vec3_add(gs_vec3_add(from, nxext), nyext); /*c*/
    p.verts[3] = gs_vec3_add(gs_vec3_add(from, nxext), yext); /*d*/
    p.verts[4] = to; /*r*/
    return p;
}

void gsi_pyramid(gs_immediate_draw_t* gsi, gs_poly_t* p, gs_color_t color, gs_graphics_primitive_type type)
{
 	// Draw square
	gsi_trianglevx(gsi, p->verts[0], p->verts[2], p->verts[1], gs_v2s(0.f), gs_v2s(1.f), gs_v2s(1.f), color, type);
	gsi_trianglevx(gsi, p->verts[2], p->verts[0], p->verts[3], gs_v2s(0.f), gs_v2s(1.f), gs_v2s(1.f), color, type);

	gsi_trianglevx(gsi, p->verts[0], p->verts[1], p->verts[4], gs_v2s(0.f), gs_v2s(1.f), gs_v2s(1.f), color, type);
	gsi_trianglevx(gsi, p->verts[1], p->verts[2], p->verts[4], gs_v2s(0.f), gs_v2s(1.f), gs_v2s(1.f), color, type);
	gsi_trianglevx(gsi, p->verts[2], p->verts[3], p->verts[4], gs_v2s(0.f), gs_v2s(1.f), gs_v2s(1.f), color, type);
	gsi_trianglevx(gsi, p->verts[3], p->verts[0], p->verts[4], gs_v2s(0.f), gs_v2s(1.f), gs_v2s(1.f), color, type);

	// gs_color_t lc = gs_color_alpha(GS_COLOR_GREEN, 255);
	// gsi_line3Dv(gsi, p->verts[0], p->verts[1], lc);
	// gsi_line3Dv(gsi, p->verts[1], p->verts[2], lc);
	// gsi_line3Dv(gsi, p->verts[2], p->verts[3], lc);
	// gsi_line3Dv(gsi, p->verts[3], p->verts[0], lc);

	// // Draw tetraherdron
	// gsi_line3Dv(gsi, p->verts[0], p->verts[4], lc);
	// gsi_line3Dv(gsi, p->verts[1], p->verts[4], lc);
	// gsi_line3Dv(gsi, p->verts[2], p->verts[4], lc);
	// gsi_line3Dv(gsi, p->verts[3], p->verts[4], lc);
}



