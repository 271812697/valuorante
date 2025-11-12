
struct Vector3
{
	FLOAT x, y, z;

	bool IsNearlyZero(float Tolerance) const
	{
		return fabs(x) <= Tolerance && fabs(y) <= Tolerance && fabs(z) <= Tolerance;
	}
};

struct Vector4
{
	float x, y, w, h;
};

struct MATRIX
{
	union
	{
		struct
		{
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;

		};

		float m[4][4];
	};
};

struct FTransform
{
public:
	Vector4 Rotation;
	Vector3 Translation;
	char Pad_1[0x4];
	Vector3 Scale3D;
	char Pad_2[0x4];
	MATRIX ToMatrixWithScale()
	{
		MATRIX m;
		m._41 = Translation.x;
		m._42 = Translation.y;
		m._43 = Translation.z;

		float x2 = Rotation.x + Rotation.x;
		float y2 = Rotation.y + Rotation.y;
		float z2 = Rotation.w + Rotation.w;

		float xx2 = Rotation.x * x2;
		float yy2 = Rotation.y * y2;
		float zz2 = Rotation.w * z2;
		m._11 = (1.0f - (yy2 + zz2)) * Scale3D.x;
		m._22 = (1.0f - (xx2 + zz2)) * Scale3D.y;
		m._33 = (1.0f - (xx2 + yy2)) * Scale3D.z;

		float yz2 = Rotation.y * z2;
		float wx2 = Rotation.h * x2;
		m._32 = (yz2 - wx2) * Scale3D.z;
		m._23 = (yz2 + wx2) * Scale3D.y;

		float xy2 = Rotation.x * y2;
		float wz2 = Rotation.h * z2;
		m._21 = (xy2 - wz2) * Scale3D.y;
		m._12 = (xy2 + wz2) * Scale3D.x;

		float xz2 = Rotation.x * z2;
		float wy2 = Rotation.h * y2;
		m._31 = (xz2 + wy2) * Scale3D.z;
		m._13 = (xz2 - wy2) * Scale3D.x;

		m._14 = 0.0f;
		m._24 = 0.0f;
		m._34 = 0.0f;
		m._44 = 1.0f;

		return m;
	}

};

MATRIX MatrixMultiplication(MATRIX pM1, MATRIX pM2)
{
	MATRIX pOut;
	pOut._11 = pM1._11 * pM2._11 + pM1._12 * pM2._21 + pM1._13 * pM2._31 + pM1._14 * pM2._41;
	pOut._12 = pM1._11 * pM2._12 + pM1._12 * pM2._22 + pM1._13 * pM2._32 + pM1._14 * pM2._42;
	pOut._13 = pM1._11 * pM2._13 + pM1._12 * pM2._23 + pM1._13 * pM2._33 + pM1._14 * pM2._43;
	pOut._14 = pM1._11 * pM2._14 + pM1._12 * pM2._24 + pM1._13 * pM2._34 + pM1._14 * pM2._44;
	pOut._21 = pM1._21 * pM2._11 + pM1._22 * pM2._21 + pM1._23 * pM2._31 + pM1._24 * pM2._41;
	pOut._22 = pM1._21 * pM2._12 + pM1._22 * pM2._22 + pM1._23 * pM2._32 + pM1._24 * pM2._42;
	pOut._23 = pM1._21 * pM2._13 + pM1._22 * pM2._23 + pM1._23 * pM2._33 + pM1._24 * pM2._43;
	pOut._24 = pM1._21 * pM2._14 + pM1._22 * pM2._24 + pM1._23 * pM2._34 + pM1._24 * pM2._44;
	pOut._31 = pM1._31 * pM2._11 + pM1._32 * pM2._21 + pM1._33 * pM2._31 + pM1._34 * pM2._41;
	pOut._32 = pM1._31 * pM2._12 + pM1._32 * pM2._22 + pM1._33 * pM2._32 + pM1._34 * pM2._42;
	pOut._33 = pM1._31 * pM2._13 + pM1._32 * pM2._23 + pM1._33 * pM2._33 + pM1._34 * pM2._43;
	pOut._34 = pM1._31 * pM2._14 + pM1._32 * pM2._24 + pM1._33 * pM2._34 + pM1._34 * pM2._44;
	pOut._41 = pM1._41 * pM2._11 + pM1._42 * pM2._21 + pM1._43 * pM2._31 + pM1._44 * pM2._41;
	pOut._42 = pM1._41 * pM2._12 + pM1._42 * pM2._22 + pM1._43 * pM2._32 + pM1._44 * pM2._42;
	pOut._43 = pM1._41 * pM2._13 + pM1._42 * pM2._23 + pM1._43 * pM2._33 + pM1._44 * pM2._43;
	pOut._44 = pM1._41 * pM2._14 + pM1._42 * pM2._24 + pM1._43 * pM2._34 + pM1._44 * pM2._44;

	return pOut;
}

namespace Bone
{
	enum Bone : INT
	{
		头部 = 32,
		颈部 = 31,
		胸部 = 4,
		肚子 = 3,
		脊椎 = 2,
		骨盆 = 1,
		左肩 = 34,
		左肘 = 35,
		左手 = 38,
		左腿 = 58,
		左膝 = 59,
		左脚跟 = 60,
		右肩 = 6,
		右肘 = 7,
		右手 = 10,
		右腿 = 62,
		右膝 = 63,
		右脚跟 = 64,
	};
	std::list<INT> _上部 = { Bone::头部 };
	std::list<INT> _右臂 = { Bone::颈部, Bone::右肩, Bone::右肘, Bone::右手 };
	std::list<INT> _左臂 = { Bone::颈部, Bone::左肩, Bone::左肘, Bone::左手 };
	std::list<INT> _脊柱 = { Bone::颈部, Bone::胸部, Bone::肚子, Bone::脊椎, Bone::骨盆 };
	std::list<INT> _右腿 = { Bone::骨盆, Bone::右腿, Bone::右膝, Bone::右脚跟 };
	std::list<INT> _左腿 = { Bone::骨盆, Bone::左腿, Bone::左膝, Bone::左脚跟 };
	std::list<std::list<INT>> 拼接骨骼 = { _上部, _右臂, _左臂, _脊柱, _右腿, _左腿 };
	std::list<INT> _四肢 = { Bone::左肘, Bone::左手, Bone::左膝, Bone::左脚跟,Bone::右肘, Bone::右手, Bone::右膝, Bone::右脚跟 };
	std::list<INT> _双脚 = { Bone::左膝, Bone::左脚跟, Bone::右膝, Bone::右脚跟 };
}

constexpr int KEY_BONES[] =
{
	Bone::头部, Bone::颈部, Bone::胸部, Bone::肚子,
	Bone::脊椎, Bone::骨盆, Bone::左肩, Bone::左肘,
	Bone::左手, Bone::右肩, Bone::右肘, Bone::右手,
	Bone::左腿, Bone::左膝, Bone::左脚跟, Bone::右腿,
	Bone::右膝, Bone::右脚跟
};
constexpr int KEY_BONE_COUNT =  sizeof(KEY_BONES) / sizeof(int);

INT GetArmorLevel(BYTE Grade)
{
	INT Level = 0;
	switch (Grade)
	{
	case 105:
	{
		Level = 1;
		break;
	}
	case 106:
	{
		Level = 1;
		break;
	}
	case 107:
	{
		Level = 1;
		break;
	}
	case 108:
	{
		Level = 1;
		break;
	}
	case 81:
	{
		Level = 2;
		break;
	}
	case 82:
	{
		Level = 2;
		break;
	}
	case 83:
	{
		Level = 2;
		break;
	}
	case 84:
	{
		Level = 2;
		break;
	}
	case 57:
	{
		Level = 3;
		break;
	}
	case 58:
	{
		Level = 3;
		break;
	}
	case 59:
	{
		Level = 3;
		break;
	}
	case 60:
	{
		Level = 3;
		break;
	}
	case 33:
	{
		Level = 4;
		break;
	}
	case 34:
	{
		Level = 4;
		break;
	}
	case 35:
	{
		Level = 4;
		break;
	}
	case 36:
	{
		Level = 4;
		break;
	}
	case 37:
	{
		Level = 4;
		break;
	}
	case 39:
	{
		Level = 4;
		break;
	}
	case 40:
	{
		Level = 4;
		break;
	}
	case 9:
	{
		Level = 5;
		break;
	}
	case 10:
	{
		Level = 5;
		break;
	}
	case 11:
	{
		Level = 5;
		break;
	}
	case 12:
	{
		Level = 5;
		break;
	}
	case 18:
	{
		Level = 5;
		break;
	}
	case 17:
	{
		Level = 5;
		break;
	}
	case 242:
	{
		Level = 6;
		break;
	}
	case 243:
	{
		Level = 6;
		break;
	}
	case 244:
	{
		Level = 6;
		break;
	}
	}

	return Level;
}

