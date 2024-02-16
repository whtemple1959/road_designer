uniform float rotVal;	//Angle of rotation for lens
uniform float innerRad;	//Lens inner radius (worldspace)
uniform float outerRad;	//Lens outer radius (worldspace)
uniform vec3 axisPoint;	//One endpoint of break-out axis
uniform vec3 midPoint;	//Midpoint of break-out axis

void main() {

    gl_TexCoord[0] = gl_MultiTexCoord0;

    if (distance(midPoint,gl_Vertex.xyz)>outerRad) {
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    }
    else {

        float vertDist=distance(midPoint,gl_Vertex.xyz);
        float rotValInterp=rotVal;

        if (vertDist>innerRad) {
            float interpDist=clamp(1.0-(vertDist-innerRad)/(outerRad-innerRad),0.0,1.0);
            rotValInterp=interpDist*rotVal;
        }

        float sinAngle=sin(rotValInterp);
        float cosAngle=cos(rotValInterp);
        float oneSubCos=1.0-cosAngle;

        vec3 rotMat1,rotMat2,rotMat3;
        vec3 aPN = normalize(axisPoint);

        rotMat1.x = aPN.x * aPN.x + cosAngle * (1.0 - aPN.x * aPN.x);
        rotMat1.y = aPN.x * aPN.y * (oneSubCos) - sinAngle * aPN.z;
        rotMat1.z = aPN.x * aPN.z * (oneSubCos) + sinAngle * aPN.y;

        rotMat2.x = aPN.x * aPN.y * (oneSubCos) + sinAngle * aPN.z;
        rotMat2.y = aPN.y * aPN.y + cosAngle * (1.0 - aPN.y * aPN.y);
        rotMat2.z = aPN.y * aPN.z * (oneSubCos) - sinAngle * aPN.x;

        rotMat3.x = aPN.x * aPN.z * (oneSubCos) - sinAngle * aPN.y;
        rotMat3.y = aPN.y * aPN.z * (oneSubCos) + sinAngle * aPN.x;
        rotMat3.z = aPN.z * aPN.z + cosAngle * (1.0 - aPN.z * aPN.z);

        vec3 temp = gl_Vertex.xyz - midPoint;      

        gl_Position.x = dot(rotMat1, temp);
        gl_Position.y = dot(rotMat2, temp);
        gl_Position.z = dot(rotMat3, temp);

        gl_Position.xyz = gl_Position.xyz + midPoint;
        gl_Position.w = 1.0;

        gl_Position = gl_ModelViewProjectionMatrix * gl_Position;

    }

} 

