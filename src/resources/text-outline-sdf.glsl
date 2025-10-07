#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 outline_color;
uniform float smoothing_param;
uniform float outline_width_param;

const float sampleSpace = 256.0;

void main()
{
    float smoothing = smoothing_param/sampleSpace;
    float outlineWidth = outline_width_param/sampleSpace;
    float outerEdgeCenter = 0.5 - outlineWidth;

    vec4  sampled_color = texture(texture0, fragTexCoord);
    float dist = sampled_color.a;
    float alpha = smoothstep(outerEdgeCenter - smoothing, outerEdgeCenter + smoothing, dist);
    float border = smoothstep(0.5-smoothing, 0.5+smoothing, dist);
    vec3 rgb = mix(outline_color, fragColor.rgb, border);

    finalColor = vec4(rgb, alpha);
}

