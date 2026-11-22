using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Collections;
using System.Collections.Generic;
using Mathf;
using Api = Nebula.Game.NebulaApiV1;
namespace Nebula
{
    namespace Game
    {
        [NativeCppClass]
        [StructLayout(LayoutKind.Sequential)]
        public struct EntityId
        {
            public UInt64 id;
        }
        public class Entity
        {
            private World world = null;
            private UInt64 id = 0xFFFFFFFFFFFFFFFF;
            private List<Property> properties;
            private MsgDispatcher dispatcher = new MsgDispatcher();
            public World World { get { return this.world; } }
            public UInt64 Id { get { return this.id; } }
            internal Entity(World world, UInt64 entityId)
            {
                this.world = world;
                this.id = entityId;
                this.properties = new List<Property>();
            }
            public bool IsValid() { return Api.IsValid(this.id); }
            public bool HasComponent<T>() where T : struct, NativeComponent
            {
                uint componentId = ComponentManager.Instance.GetComponentId<T>();
                return Api.HasComponent(this.id, componentId);
            }
            public void SetComponent<T>(in T component) where T : struct, NativeComponent
            {
                uint componentId = ComponentManager.Instance.GetComponentId<T>();
                int size = Marshal.SizeOf<T>();
                IntPtr ptr = Marshal.AllocHGlobal(size);
                Marshal.StructureToPtr(component, ptr, false);
                Api.SetComponentData(this.id, componentId, ptr, size);
                Marshal.FreeHGlobal(ptr);
            }
            public T GetComponent<T>() where T : struct, NativeComponent
            {
                uint componentId = ComponentManager.Instance.GetComponentId<T>();
                int size = Marshal.SizeOf<T>();
                IntPtr ptr = Marshal.AllocHGlobal(size);
                Api.GetComponentData(this.id, componentId, ptr, size);
                T component = Marshal.PtrToStructure<T>(ptr);
                Marshal.FreeHGlobal(ptr);
                return component;
            }
            public bool HasProperty<T>() where T : Property
            {
                for (int i = 0; i < this.properties.Count; i++)
                {
                    if (this.properties[i] is T)
                    {
                        return true;
                    }
                }
                return false;
            }
            public void AddProperty(Property property)
            {
                Debug.Assert(this.id != 0xFFFFFFFF);
                property.Entity = this;
                this.properties.Add(property);
                PropertyManager.Instance.RegisterProperty(property);
                property.Active = true;
                this.dispatcher.AttachHandler(property.OnMessage, property.AcceptedMessages());
            }
            public T GetProperty<T>() where T : Property
            {
                for (int i = 0; i < this.properties.Count; i++)
                {
                    if (this.properties[i] is T)
                    {
                        return this.properties[i] as T;
                    }
                }
                return null;
            }
            public Vector3 GetPosition()
            {
                return Api.GetPosition(this.id);
            }
            public void SetPosition(Vector3 position)
            {
                Api.SetPosition(this.id, position);
            }
            public Quaternion GetOrientation()
            {
                return Api.GetOrientation(this.id);
            }
            public void SetOrientation(Quaternion orientation)
            {
                Api.SetOrientation(this.id, orientation);
            }
            public Vector3 GetScale()
            {
                return Api.GetScale(this.id);
            }
            public void SetScale(Vector3 scale)
            {
                Api.SetScale(this.id, scale);
            }
            public void Send<T>(in T msg) where T : struct, Msg
            {
                this.dispatcher.Dispatch(msg);
            }
            public static void Destroy(Entity entity)
            {
                if (entity.id == 0xFFFFFFFFFFFFFFFF)
                    return;
                Api.DeleteEntity(entity.id);
                entity.id = 0xFFFFFFFFFFFFFFFF;
                for (int i = 0; i < entity.properties.Count; i++)
                {
                    Property property = entity.properties[i];
                    property.Destroy();
                }
                entity.properties.Clear();
            }
            public override string ToString()
            {
                return id.ToString();
            }
        }
    }
}