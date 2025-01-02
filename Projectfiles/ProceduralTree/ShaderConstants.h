#pragma once
const char* VERTEX_SHADER_TF = R"(
	#version 330
	
	uniform mat4 transformation;

	layout (location = 0) in vec4 in_position;
	layout (location = 1) in vec4 in_color;
	layout (location = 2) in vec4 in_normal;
	layout (location = 3) in float in_length;

	out vec4 var_position;		
	out vec4 var_color;
	out vec4 var_normal;
	out float var_length;
			
	void main()
	{
		var_position = in_position;
				
		var_color = in_color;
		var_normal = in_normal;
		var_length = in_length;
	}

)";


const char* VERTEX_SHADER_RE = R"(
	#version 330
	
	uniform mat4 transformation;

	layout (location = 0) in vec4 in_position;
	layout (location = 1) in vec4 in_color;
	
		
	out vec4 var_color;
	
	
	void main()
	{
		gl_Position = transformation * in_position;	
		var_color = in_color;
	
	}
	
)";

const char* GEOMETRY_SHADER = R"(
	#version 330
	
	uniform mat4 transformation;
	layout (triangles) in;
	layout (triangle_strip, max_vertices = 33) out;
	
	in vec4 var_position[];
	in vec4 var_color[];
	in vec4 var_normal[];
	in float var_length[];
	
	out vec4  outGS_position;
	out vec4  outGS_color;
	out vec4  outGS_normal;
	out float outGS_length;

	void main(){

		
		for(int i = 0; i < 3; ++i) {
				outGS_position = gl_Position = var_position[i];
				outGS_color = var_color[i];
				outGS_normal = var_normal[i];
				outGS_length = 0;
				EmitVertex();
			
        }
        EndPrimitive();
		
		if(var_length[0] > 0.0f){
			vec4 new_vec[3];

			vec3 center = vec3(0.0f, 0.0f,0.0f);
				for(int i=0; i<3; ++i){
					center.x += var_position[i].x;
					center.y += var_position[i].y;
					center.z += var_position[i].z;
				}
			
				center.x /= 3.0f;
				center.y /= 3.0f;
				center.z /= 3.0f;
			for(int i = 0; i < 3; ++i){

					
					vec4 newPosition = var_position[i] + var_length[i] * var_normal[i];
				

					newPosition = vec4(center, 1.0f) + 0.3f * (newPosition - vec4(center, 1.0f));
					newPosition /= newPosition.w; 
					outGS_position = new_vec[i] = gl_Position = newPosition;
					outGS_color = var_color[i] * vec4(0.7f, 0.8f, 0.2f, 1.0f);
					outGS_normal = var_normal[i];
					outGS_length = 0.0f;
					EmitVertex();
			}
			EndPrimitive();	


			for(int i = 0; i < 3; ++i){
				outGS_position = gl_Position = new_vec[i];
				outGS_color = vec4(0.7f, 0.8f, 0.2f, 1.0f);
				outGS_normal =var_normal[i];
				outGS_length = 0.0f;
				EmitVertex();
				
				for(int j = i; j < i+2; ++j){
					int index = j % 3;
					outGS_position = gl_Position = var_position[index];
					outGS_color = var_color[0] * vec4(0.7f, 0.8f, 0.2f, 1.0f);
					outGS_normal = var_normal[i];
					outGS_length = 0.0f;
					EmitVertex();
			
				}
				EndPrimitive();
			}

			for(int i = 0; i < 3; ++i){
				outGS_position = gl_Position = var_position[i];
				outGS_color = vec4(0.0f, 0.8f, 0.2f, 1.0f);
				outGS_normal = var_normal[i];
				outGS_length = 0.0f;
				EmitVertex();
				
				for(int j = i; j > i-2; --j){
					int index = (j+3) % 3;
					outGS_position = gl_Position = new_vec[index];
					outGS_color = var_color[0] * vec4(0.0f, 0.8f, 0.2f, 1.0f);
					outGS_normal = var_normal[index];
					outGS_length = 0.0f;
					EmitVertex();
			
				}
				EndPrimitive();
			}


	
			center = vec3(0.0f, 0.0f,0.0f);
			for(int i=0; i<3; ++i){
				center.x += new_vec[i].x;
				center.y += new_vec[i].y;
				center.z += new_vec[i].z;
			}
			
			center.x /= 3.0f;
			center.y /= 3.0f;
			center.z /= 3.0f;

			vec4 top_vec = vec4(center, 1.0f) + 0.1f * var_normal[0];
			top_vec /= top_vec.w;
			
			for(int i = 0; i < 3; ++i){
			
				int index1 = i % 3;
				int index2 = (i+1) % 3;
				 
				vec3 normal = normalize(cross(new_vec[index2].xyz - top_vec.xyz, new_vec[index1].xyz - top_vec.xyz));

				outGS_position = gl_Position = top_vec;
				outGS_color = vec4(0.7f, 0.8f, 0.2f, 1.0f);
				outGS_normal = vec4(normal, 1.0f);
				float newLength = var_length[i] - 0.3f;
				if(newLength <= 0.0f){
					outGS_length = 0.0f;
				}else{
					outGS_length = var_length[i] - 0.3f;

				}
				EmitVertex();


				outGS_position = gl_Position = new_vec[index1];
				outGS_color = vec4(0.7f, 0.2f, 0.0f, 1.0f);
				outGS_normal = vec4(normal, 1.0f);
				if(newLength <= 0.0f){
					outGS_length = 0.0f;
				}else{
					outGS_length = var_length[i] - 0.3f;

				}
				EmitVertex();

				outGS_position = gl_Position = new_vec[index2];
				outGS_color = vec4(0.7f, 0.2f, 0.0f, 1.0f);
				outGS_normal = vec4(normal, 1.0f);
				if(newLength <= 0.0f){
					outGS_length = 0.0f;
				}else{
					outGS_length = var_length[i] - 0.3f;

				}
				EmitVertex();				
				
				EndPrimitive();	
		
				
			}
		}
	}

)";

const char* FRAGMENT_SHADER = R"(
	#version 330
	
	in vec4 var_color;

	out vec4 out_color;

	void main()
	{
		out_color = var_color;
	}	

)";