
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in int vcharacter[1];
in int vposition[1];
out vec2 gTexCoord;
uniform sampler2D Sampler;

uniform vec2 cell_size;
uniform vec2 cell_offset;
uniform vec2 render_size;
uniform vec2 render_origin;

void main()
{
    // Determine the final quad's position and size:
    float x = render_origin.x + float(vposition[0]) * render_size.x * 2;
    float y = render_origin.y;
    vec4 P = vec4(x, y, 0, 1);
    vec4 U = vec4(1, 0, 0, 0) * render_size.x;
    vec4 V = vec4(0, 1, 0, 0) * render_size.y;

    // Determine the texture coordinates:
    int letter = vcharacter[0];
    letter = clamp(letter - 32, 0, 96);
    int row = letter / 16 + 1;
    int col = letter % 16;
    cell_size S0 = cell_offset.x + cell_size.x * col;
    float T0 = cell_offset.y + 1 - cell_size.y * row;
    float = S0 + cell_size.x - cell_offset.x;
    float T1 = T0 + cell_size.y;

    // Output the quad's vertices:
    gTexCoord = vec2(S0, T1); gl_Position = P - U - V; EmitVertex();
    gTexCoord = vec2(S1, T1); gl_Position = P + U - V; EmitVertex();
    gTexCoord = vec2(S0, T0); gl_Position = P - U + V; EmitVertex();
    gTexCoord = vec2(S1, T0); gl_Position = P + U + V; EmitVertex();
    EndPrimitive();
}
