using System;
using System.Runtime.CompilerServices;
using Vault;

namespace Vault
{
    public class Transform : Component
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void GetKey(string key, string id, out string result);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void SetKey(string key, string id, float x, float y, float z);

        public Vector3 position
        {
            get
            {
                GetKey("position", Entity.ID, out string result);
                string[] strValues = result.Split(" ");
                return new Vector3(float.Parse(strValues[0]), float.Parse(strValues[1]), float.Parse(strValues[2]));
            }
            set
            {
                SetKey("position", Entity.ID, value.x, value.y, value.z);
            }
        }

        public Vector3 rotation
        {
            get
            {
                GetKey("rotation", Entity.ID, out string result);
                string[] strValues = result.Split(" ");
                return new Vector3(float.Parse(strValues[0]), float.Parse(strValues[1]), float.Parse(strValues[2]));
            }
            set
            {
                SetKey("rotation", Entity.ID, value.x, value.y, value.z);
            }
        }

        public Vector3 scale
        {
            get
            {
                GetKey("scale", Entity.ID, out string result);
                string[] strValues = result.Split(" ");
                return new Vector3(float.Parse(strValues[0]), float.Parse(strValues[1]), float.Parse(strValues[2]));
            }
            set
            {
                SetKey("scale", Entity.ID, value.x, value.y, value.z);
            }
        }

        public Vector3 forward
        {
            get
            {
                GetKey("forward", Entity.ID, out string result);
                string[] strValues = result.Split(" ");
                return new Vector3(float.Parse(strValues[0]), float.Parse(strValues[1]), float.Parse(strValues[2]));
            }
            set { }
        }
    }
}