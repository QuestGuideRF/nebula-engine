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
        public class EntityTemplate
        {
            public string name;
        }
        public sealed class TemplateManager
        {
            private delegate Property DeserializeProperty(string json);
            private class Template
            {
                private class PropertyData
                {
                    public string json;
                    public DeserializeProperty Deserialize;
                }
                List<PropertyData> data;
            }
            private Dictionary<string, DeserializeProperty> deserializeFuncs = new Dictionary<string, DeserializeProperty>();
            private Dictionary<string, Template> templates = new Dictionary<string, Template>();
            private static readonly TemplateManager instance = new TemplateManager();
            static TemplateManager() { }
            private TemplateManager()
            {
            }
            public static TemplateManager Instance
            {
                get
                {
                    return instance;
                }
            }
            public void SetupTemplates(string path)
            {
                string[] files = System.IO.Directory.GetFiles(path, "*.json", System.IO.SearchOption.AllDirectories);
                foreach (string file in files)
                {
                }
            }
            public void RegisterPropertyType<T>() where T : Property
            {
                string name = typeof(T).Name;
                Property DeserializePropertyFunc<TYPE>(string json) => System.Text.Json.JsonSerializer.Deserialize<TYPE>(json) as Property;
                DeserializeProperty deserialize = DeserializePropertyFunc<T>;
                this.deserializeFuncs.Add(name, deserialize);
            }
        }
    }
}