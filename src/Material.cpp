#include "Material.h"

#include <d3d11.h>

void Material::BindResources(ID3D11DeviceContext* Context) const
{
	RenderPipeline->Bind(Context);
}
