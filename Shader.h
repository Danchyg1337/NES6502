#pragma once
#include <sstream>
#include <iostream>

namespace primitive {
	float square[] = {
			-1.0f,  1.0f,  0.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			 1.0f,  1.0f,  1.0f, 1.0f,
			 1.0f,  1.0f,  1.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			-1.0f, -1.0f,  0.0f, 0.0f
	};
}

struct ShaderSource {
	std::string vertex;
	std::string fragment;
	std::string geometry;
};

enum ShaderType {
	NONE = -1, VERTEX, FRAGMENT, GEOMETRY
};

class Shader {
public:
	mutable std::unordered_map<const char*, GLuint> shaderUniformLocations;
	unsigned int Program;
	Shader() = default;
	Shader(const char* ShaderPath) {
		std::ifstream file(ShaderPath);
		std::string line;
		std::stringstream ss[3];
		ShaderType type = ShaderType::NONE;

		if (!file.good()) {
			std::cout << "SHADER::ERROR::WRONGPATH\n";
			return;
		}

		while (getline(file, line)) {
			if (line.find("#shader") != std::string::npos) {
				if (line.find("vertex") != std::string::npos) {
					type = ShaderType::VERTEX;
				}
				if (line.find("fragment") != std::string::npos) {
					type = ShaderType::FRAGMENT;
				}
				if (line.find("geometry") != std::string::npos) {
					type = ShaderType::GEOMETRY;
				}
			}
			else {
				ss[(int)type] << line << "\n";
			}
		}
		ShaderSource shs = { ss[0].str(), ss[1].str(), ss[2].str() };
		this->Program = CreateShader(shs.vertex, shs.fragment, shs.geometry);
	}

	GLuint SetUniform(const char* name) const {
		if (shaderUniformLocations.find(name) != shaderUniformLocations.end())
			return shaderUniformLocations[name];
		GLuint loc = glGetUniformLocation(Program, name);
		shaderUniformLocations[name] = loc;
		return loc;
	}

	void Use() {
		glUseProgram(this->Program);
	}

	void DeleteShader() {
		glDeleteProgram(this->Program);
	}

private:
	int CompileShader(std::string& source, unsigned int type) {
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int res;
		glGetShaderiv(id, GL_COMPILE_STATUS, &res);
		if (res == GL_FALSE) {
			int len;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
			char* message = (char*)alloca(len * sizeof(char));
			glGetShaderInfoLog(id, len, &len, message);
			std::cout << message << std::endl;
		}


		return id;
	}

	int CreateShader(std::string& vshader, std::string& fshader, std::string& gshader) {
		unsigned int program = glCreateProgram();
		unsigned int vs, fs, gs;

		if (!vshader.empty()) {
			vs = CompileShader(vshader, GL_VERTEX_SHADER);
			glAttachShader(program, vs);
		}
		if (!fshader.empty()) {
			fs = CompileShader(fshader, GL_FRAGMENT_SHADER);
			glAttachShader(program, fs);
		}
		if (!gshader.empty()) {
			gs = CompileShader(gshader, GL_GEOMETRY_SHADER);
			glAttachShader(program, gs);
		}
		glLinkProgram(program);
		glValidateProgram(program);

		int res;
		glGetProgramiv(program, GL_LINK_STATUS, &res);
		if (res == GL_FALSE) {
			int len;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
			char* message = (char*)alloca(sizeof(char) * len);
			glGetProgramInfoLog(program, len, &len, message);
			std::cout << message << std::endl;
		}


		if (!vshader.empty()) glDeleteShader(vs);
		if (!fshader.empty()) glDeleteShader(fs);
		if (!gshader.empty()) glDeleteShader(gs);
		return program;
	}
};
