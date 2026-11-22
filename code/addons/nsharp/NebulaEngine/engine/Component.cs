using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Collections;
using System.Collections.Generic;
using Mathf;
using Nebula.Game;
namespace Nebula
{
    namespace Game
    {
        public class ComponentManager
        {
            private Dictionary<Type, uint> componentRegistry = new Dictionary<Type, uint>();
            private static readonly ComponentManager instance = new ComponentManager();
            static ComponentManager() { }
            private ComponentManager()
            {
            }
            public static ComponentManager Instance
            {
                get
                {
                    return instance;
                }
            }
            public uint GetComponentId<T>() where T : NativeComponent
            {
                uint id = 0xFFFFFFFF;
                bool res = componentRegistry.TryGetValue(typeof(T), out id);
                Debug.Assert(res, "Не удалось найти идентификатор компонента указанного типа!");
                return id;
            }
            public void RegisterComponent(Type type, uint id)
            {
                this.componentRegistry[type] = id;
            }
        }
        public interface NativeComponent
        {
        }
        [NativeCppClass]
        [StructLayout(LayoutKind.Sequential)]
        public struct ResourceDescriptor
        {
            public readonly IntPtr descriptor;
        }
    }
}