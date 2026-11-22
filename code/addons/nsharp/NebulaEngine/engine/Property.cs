using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Collections;
using System.Collections.Generic;
using Mathf;
using Nebula;
using Nebula.Game;
namespace Nebula
{
    namespace Game
    {
        public enum FrameEvent
        {
            OnBeginFrame,
            OnFixedFrame,
            OnFrame,
            OnEndFrame,
            NumEvents
        }
        public class Property
        {
            private bool active = false;
            private Entity entity = null;
            public bool Active
            {
                get { return active; }
                set
                {
                    if (active == value)
                    {
                        return;
                    }
                    if (value)
                    {
                        this.OnActivate();
                    }
                    else
                    {
                        this.OnDeactivate();
                    }
                    active = value;
                }
            }
            public Entity Entity
            {
                get
                {
                    return entity;
                }
                set
                {
                    if (entity == null)
                    {
                        entity = value;
                    }
                    else
                    {
                        Debug.Log("Предупреждение. Попытка изменить владельца объекта недвижимости, который уже принадлежит!");
                    }
                }
            }
            public bool IsValid { get { return entity != null; } }
            public virtual Nebula.Game.FrameEvent[] AcceptedEvents()
            {
                return null;
            }
            public virtual System.Type[] AcceptedMessages()
            {
                return null;
            }
            public virtual void OnMessage(in Msg msg)
            {
            }
            public override string ToString()
            {
                if (entity != null)
                {
                    return GetType().ToString() + " -> Сущность: " + entity.ToString();
                }
                return GetType().ToString();
            }
            public virtual void OnActivate() { }
            public virtual void OnDeactivate() { }
            public virtual void OnBeginFrame() { }
            public virtual void OnFrame() { }
            public virtual void OnEndFrame() { }
            public void Destroy()
            {
                if (IsValid && active)
                {
                    this.OnDeactivate();
                    entity = null;
                }
            }
        }
    }
}