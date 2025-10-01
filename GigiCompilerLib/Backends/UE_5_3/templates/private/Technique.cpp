#include "/*$(Name)*/Technique.h"
#include "ScenePrivate.h"

/*$(TechniqueCPP_CVars)*/namespace /*$(Name)*/
{

template <typename T>
T Pow2GE(const T& A)
{
    float f = log2(float(A));
    f = ceilf(f);
    return (T)pow(2.0f, f);
}

/*$(TechniqueCPPNamespace)*//*$(InitializeFnCPP)*//*$(TechniqueCPP_Shaders)*/void AddTechnique(FRDGBuilder& GraphBuilder, const FViewInfo& View, FTechniqueParams& params)
{
    if (!View.ViewState)
        return;

/*$(VariableMaintenance)*//*$(SetVarBefore)*//*$(AddTechnique)*//*$(SetVarAfter)*//*$(AddTechniqueSetExports)*/}

};
