using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Collections;
using System.Collections.Generic;
using Mathf;
using System.Reflection;
using System.Linq;
namespace Nebula
{
    namespace Game
    {
        public sealed class PropertyManager
        {
            private List<Property>[] eventListeners = new List<Property>[(int)Nebula.Game.FrameEvent.NumEvents];
            private static readonly PropertyManager instance = new PropertyManager();
            static PropertyManager()
            {
            }
            private PropertyManager()
            {
                for (int i = 0; i < this.eventListeners.Length; i++)
                {
                    this.eventListeners[i] = new List<Property>();
                }
            }
            public static PropertyManager Instance
            {
                get
                {
                    return instance;
                }
            }
            public void RegisterProperty(Property property)
            {
                FrameEvent[] acceptedEvents = property.AcceptedEvents();
                if (acceptedEvents != null)
                {
                    for (int i = 0; i < acceptedEvents.Length; i++)
                    {
                        this.eventListeners[(int)acceptedEvents[i]].Add(property);
                    }
                }
            }
            public void OnBeginFrame()
            {
                List<Property> props = this.eventListeners[(int)FrameEvent.OnBeginFrame];
                for (int i = 0; i < props.Count; i++)
                {
                    Property prop = props[i];
                    if (!prop.IsValid)
                    {
                        props.EraseSwap(i);
                        i--;
                        continue;
                    }
                    if (prop.Active)
                        prop.OnBeginFrame();
                }
            }
            public void OnFrame()
            {
                List<Property> props = this.eventListeners[(int)FrameEvent.OnFrame];
                for (int i = 0; i < props.Count; i++)
                {
                    Property prop = props[i];
                    if (!prop.IsValid)
                    {
                        props.EraseSwap(i);
                        i--;
                        continue;
                    }
                    if (prop.Active)
                        prop.OnFrame();
                }
            }
            public void OnEndFrame()
            {
                List<Property> props = this.eventListeners[(int)FrameEvent.OnEndFrame];
                for (int i = 0; i < props.Count; i++)
                {
                    Property prop = props[i];
                    if (!prop.IsValid)
                    {
                        props.EraseSwap(i);
                        i--;
                        continue;
                    }
                    if (prop.Active)
                        prop.OnEndFrame();
                }
            }
        }
    }
}