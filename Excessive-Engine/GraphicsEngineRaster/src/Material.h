#pragma once

#include <mymath/mymath.h>
#include <vector>
#include "../GraphicsEngine_Interface/interface/IMaterial.h"

class Texture;


class Material : public ge::IMaterial
{
public:
	// ctor, dtor, release
	Material();
	~Material();

	void acquire();
	void release() override;

	// load
	void load(const char* file_path) override;
	void load(const wchar_t* file_path) override;

	void reset() override;

	// modify
	SubMaterial& addSubMaterial() override;
	int getNumSubMaterials() const override;
	void setNumSubMaterials(int n) override;
	void deleteSubMaterial(int index) override;

	SubMaterial& getSubMaterial(int index) override;
	const SubMaterial& getSubMaterial(int index) const override;

	SubMaterial& operator[](int index);
	const SubMaterial& operator[](int index) const;
private:
	size_t refcount;
	std::vector<SubMaterial> sub_materials;
};