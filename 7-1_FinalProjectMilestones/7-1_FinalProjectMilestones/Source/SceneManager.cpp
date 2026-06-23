#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <iostream>
#include <glm/gtx/transform.hpp>

namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
	m_loadedTextures = 0;

	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].ID = 0;
		m_textureIDs[i].tag = "";
	}
}

SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load(filename, &width, &height, &colorChannels, 0);

	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			stbi_image_free(image);
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;
	return false;
}

void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glDeleteTextures(1, &m_textureIDs[i].ID);
	}
}

int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return textureID;
}

int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return textureSlot;
}

bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return false;
	}

	int index = 0;
	bool bFound = false;

	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return true;
}

void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	scale = glm::scale(scaleXYZ);
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

void SceneManager::SetShaderTexture(std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

void SceneManager::SetShaderMaterial(std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material = {};
		bool bReturn = FindMaterial(materialTag, material);

		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

void SceneManager::PrepareScene()
{
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadSphereMesh();

	CreateGLTexture("../../Utilities/textures/pavers.jpg", "floor");
	CreateGLTexture("../../Utilities/textures/stainless.jpg", "metal");
	CreateGLTexture("../../Utilities/textures/gold-seamless-texture.jpg", "gold");
	CreateGLTexture("../../Utilities/textures/stainedglass.jpg", "glass");

	BindGLTextures();

	OBJECT_MATERIAL floorMaterial = {};
	floorMaterial.ambientColor = glm::vec3(0.4f, 0.4f, 0.4f);
	floorMaterial.ambientStrength = 0.12f;
	floorMaterial.diffuseColor = glm::vec3(0.45f, 0.45f, 0.45f);
	floorMaterial.specularColor = glm::vec3(0.15f, 0.15f, 0.15f);
	floorMaterial.shininess = 8.0f;
	floorMaterial.tag = "floor";
	m_objectMaterials.push_back(floorMaterial);

	OBJECT_MATERIAL metalMaterial = {};
	metalMaterial.ambientColor = glm::vec3(0.45f, 0.45f, 0.45f);
	metalMaterial.ambientStrength = 0.12f;
	metalMaterial.diffuseColor = glm::vec3(0.45f, 0.45f, 0.45f);
	metalMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.5f);
	metalMaterial.shininess = 18.0f;
	metalMaterial.tag = "metal";
	m_objectMaterials.push_back(metalMaterial);

	OBJECT_MATERIAL goldMaterial = {};
	goldMaterial.ambientColor = glm::vec3(0.45f, 0.35f, 0.15f);
	goldMaterial.ambientStrength = 0.12f;
	goldMaterial.diffuseColor = glm::vec3(0.5f, 0.4f, 0.18f);
	goldMaterial.specularColor = glm::vec3(0.5f, 0.4f, 0.2f);
	goldMaterial.shininess = 16.0f;
	goldMaterial.tag = "gold";
	m_objectMaterials.push_back(goldMaterial);

	OBJECT_MATERIAL glassMaterial = {};
	glassMaterial.ambientColor = glm::vec3(0.35f, 0.35f, 0.5f);
	glassMaterial.ambientStrength = 0.12f;
	glassMaterial.diffuseColor = glm::vec3(0.45f, 0.45f, 0.6f);
	glassMaterial.specularColor = glm::vec3(0.5f, 0.5f, 0.7f);
	glassMaterial.shininess = 18.0f;
	glassMaterial.tag = "glass";
	m_objectMaterials.push_back(glassMaterial);

	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	m_pShaderManager->setVec3Value("lightSources[0].position", glm::vec3(-5.0f, 8.0f, 8.0f));
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", glm::vec3(0.08f, 0.08f, 0.08f));
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", glm::vec3(0.35f, 0.35f, 0.35f));
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", glm::vec3(0.35f, 0.35f, 0.35f));
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 12.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.08f);

	m_pShaderManager->setVec3Value("lightSources[1].position", glm::vec3(5.0f, 6.0f, 4.0f));
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", glm::vec3(0.03f, 0.01f, 0.02f));
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", glm::vec3(0.12f, 0.04f, 0.08f));
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", glm::vec3(0.15f, 0.06f, 0.10f));
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 10.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.05f);

	m_pShaderManager->setVec3Value("lightSources[2].position", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.0f);

	m_pShaderManager->setVec3Value("lightSources[3].position", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);
}

void SceneManager::RenderScene()
{
	glm::vec3 scaleXYZ;
	glm::vec3 positionXYZ;

	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("floor");
	SetTextureUVScale(4.0f, 4.0f);
	SetShaderMaterial("floor");
	m_basicMeshes->DrawPlaneMesh();

	scaleXYZ = glm::vec3(3.0f, 5.0f, 3.0f);
	positionXYZ = glm::vec3(0.0f, 2.5f, 0.0f);

	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("metal");
	SetTextureUVScale(2.0f, 2.0f);
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	scaleXYZ = glm::vec3(2.0f, 3.5f, 2.0f);
	positionXYZ = glm::vec3(0.0f, 7.5f, 0.0f);

	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("gold");
	SetTextureUVScale(1.5f, 1.5f);
	SetShaderMaterial("gold");
	m_basicMeshes->DrawConeMesh();

	scaleXYZ = glm::vec3(1.0f, 1.0f, 1.0f);
	positionXYZ = glm::vec3(0.0f, 10.0f, 0.0f);

	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderTexture("glass");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("glass");
	m_basicMeshes->DrawSphereMesh();
}