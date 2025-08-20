#include "mesh.hpp"

mesh::mesh() {
	// gen array
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// gen buf
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// setup attrib layout
	glVertexAttribPointer(
	    0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh::vertex), (GLvoid *)0
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
	    1,
	    2,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof(mesh::vertex),
	    (GLvoid *)(3 * sizeof(float))
	);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
	    2,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof(mesh::vertex),
	    (GLvoid *)(5 * sizeof(float))
	);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(
	    3,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof(mesh::vertex),
	    (GLvoid *)(8 * sizeof(float))
	);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
mesh::~mesh() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void mesh::load(const std::vector<mesh::vertex> &verts) {
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(
	    GL_ARRAY_BUFFER,
	    verts.size() * sizeof(mesh::vertex),
	    verts.data(),
	    GL_DYNAMIC_DRAW
	);
	num_verts = verts.size();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void mesh::load_from_file(const std::filesystem::path &path) {
	// load file
	Assimp::Importer importer;
	const aiScene   *ai_scene = importer.ReadFile(
        path.string(),
        aiProcess_Triangulate | aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices
    );
	if (!ai_scene || (ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) > 0 ||
	    !ai_scene->mRootNode) {
		throw std::runtime_error(importer.GetErrorString());
	}

	// extract mesh data
	std::vector<mesh::vertex> merged_verts;
	for (uint32_t i = 0; i < ai_scene->mNumMeshes; i++) {
		aiMesh *ai_mesh = ai_scene->mMeshes[i];

		std::vector<mesh::vertex> temp_verts;
		temp_verts.reserve(ai_mesh->mNumVertices);
		for (size_t i = 0; i < ai_mesh->mNumVertices; i++) {
			aiVector3D &pos     = ai_mesh->mVertices[i];
			aiVector3D &uv      = ai_mesh->mTextureCoords[0][i];
			aiVector3D &norm    = ai_mesh->mNormals[i];
			aiVector3D &tangent = ai_mesh->mTangents[i];

			temp_verts.push_back(
			    vertex{
			        .pos     = {pos.x, pos.y, pos.z},
			        .uv      = {uv.x, uv.y},
			        .norm    = {norm.x, norm.y, norm.z},
			        .tangent = {tangent.x, tangent.y, tangent.z}
			    }
			);
		}

		merged_verts.reserve(merged_verts.size() + ai_mesh->mNumFaces * 3);
		for (size_t i = 0; i < ai_mesh->mNumFaces; i++) {
			unsigned int *indices = ai_mesh->mFaces[i].mIndices;
			for (int i = 0; i < 3; i++) {
				merged_verts.push_back(temp_verts[indices[i]]);
			}
		}
	}

	load(merged_verts);
}

void mesh::draw(GLenum draw_mode) {
	glBindVertexArray(vao);
	glDrawArrays(draw_mode, 0, num_verts);
}
