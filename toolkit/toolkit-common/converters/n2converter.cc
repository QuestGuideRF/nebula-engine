//------------------------------------------------------------------------------
//  n2converter.cc
//  (C) 2008 г. Радон Лабс ГмбХ
//  (C) 2013–2016 Индивидуальные авторы, см. файл АВТОРОВ.
//------------------------------------------------------------------------------
#include "foundation/stdneb.h"
#include "n2converter.h"
#include "toolkitutil/binarymodelwriter.h"
#include "toolkitutil/xmlmodelwriter.h"
#include "io/ioserver.h"
#include "math/point.h"
#include "math/vector.h"
#include "io/xmlreader.h"

namespace ToolkitUtil
{
using namespace Util;
using namespace IO;
using namespace Math;

//------------------------------------------------------------------------------
/**
*/
N2Converter::N2Converter() :
    srcDir("dst:gfxlib"),
    dstDir("dst:models"),
    mshDir("dst:meshes"),
    platform(Platform::Win32),
    force(false),
    binary(true),
    nodeType(InvalidNodeType),
    verbose(false),
    isValid(false)
{
    // empty
    this->animInfo.animation.Clear();
    this->animInfo.key.Clear();
    this->animInfo.loopType.Clear();
    this->animInfo.paramName.Clear();
    this->animInfo.animatorNode = InvalidAnimatorNodeType;
    this->animInfo.animationGroup = -1;
    this->animInfo.vectorName.Clear();
    this->animInfo.posKey.Clear();
    this->animInfo.eulerKey.Clear();
    this->animInfo.scaleKey.Clear();
    this->animInfo.layer.Clear();

    //this->alignmentTable.Add("", "UnalignedX");
    this->alignmentTable.Add("Left", "LeftSide");
    this->alignmentTable.Add("Center", "CenterX");
    this->alignmentTable.Add("Right", "RightSide");
    //this->alignmentTable.Add("", "UnalignedY");
    this->alignmentTable.Add("Top", "UpSide");
    this->alignmentTable.Add("VCenter", "CenterY");
    this->alignmentTable.Add("Bottom", "DownSide");    
}

//------------------------------------------------------------------------------
/**
*/
N2Converter::~N2Converter()
{
    n_assert(!this->IsValid());
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::Setup()
{
    n_assert(!this->IsValid());
    this->isValid = true;
    this->ResetUsedResources();
        
    // настройка объекта N2ReflectionInfo и N2SceneLoader
    this->n2ReflectionInfo = N2ReflectionInfo::Create();
    this->n2ReflectionInfo->Setup();
    this->n2SceneLoader = N2SceneLoader::Create();
    this->n2SceneLoader->Setup(this->n2ReflectionInfo);
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::Discard()
{
    n_assert(this->IsValid());

    this->n2SceneLoader->Discard();
    this->n2ReflectionInfo->Discard();
    this->n2SceneLoader = 0;
    this->n2ReflectionInfo = 0;
    this->isValid = false;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2Converter::IsValid() const
{
    return this->isValid;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2Converter::ConvertAll(Logger& logger)
{
    bool success = true;
    Array<String> categories = IoServer::Instance()->ListDirectories(this->srcDir, "*");
    IndexT i;
    for (i = 0; i < categories.Size(); i++)
    {
        // игнорировать каталоги системы контроля версий
        if ((categories[i] != "CVS") && (categories[i] != ".svn"))
        {
            success &= this->ConvertCategory(logger, categories[i]);
        }
    }
    return success;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2Converter::ConvertCategory(Logger& logger, const String& category)
{
    n_assert(category.IsValid());
    bool success = true;

    // построить абсолютный путь к категории и получить список текстур в категории
    String catDir;
    catDir.Format("%s/%s", this->srcDir.AsCharPtr(), category.AsCharPtr());
    Array<String> files = IoServer::Instance()->ListFiles(catDir, "*.n2");

    // for each file...
    IndexT i;
    for (i = 0; i < files.Size(); i++)
    {
        success &= this->ConvertFile(logger, category, files[i]);
    }
    return success;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2Converter::ConvertFile(Logger& logger, const String& category, const String& fileName)
{
    this->srcPath.Format("%s/%s/%s", this->srcDir.AsCharPtr(), category.AsCharPtr(), fileName.AsCharPtr());
   
    // ИСПРАВИТЬ!
    // Отладочный дамп разобранного дерева узлов сцены...
    // Это дерево узлов сцены позже станет основой для преобразования из формата файла N2 в N3.
    if (this->verbose)
    {
        Ptr<SceneNodeTree> sceneNodeTree = this->n2SceneLoader->Load(this->srcPath, logger);
        logger.Print("\n*** ДАМП: %s:\n", this->srcPath.AsCharPtr());
        sceneNodeTree->Dump(logger);
    }
    
    String dstFilename = fileName;
    dstFilename.StripFileExtension();
    if (this->binary)
    {
        dstFilename.Append(".n3");
    }
    else
    {
        dstFilename.Append(".xml");
    }
    this->dstPath.Format("%s/%s/%s", this->dstDir.AsCharPtr(), category.AsCharPtr(), dstFilename.AsCharPtr());
    
    // проверьте, необходима ли конвертация
    if (!this->NeedsConversion(srcPath, dstPath))
    {
        return true;
    }

    // статус отображения
    if (this->verbose)
    {
        logger.Print("%s -> %s\n", this->srcPath.AsCharPtr(), this->dstPath.AsCharPtr());
    }

    // убедитесь, что каталог dst существует
    IoServer* ioServer = IoServer::Instance();
    String dstCatDir;
    dstCatDir.Format("%s/%s", this->dstDir.AsCharPtr(), category.AsCharPtr());
    ioServer->CreateDirectory(dstCatDir);

    // настроить потоки ввода-вывода и потоковые считыватели/писатели
    Ptr<Stream> srcStream = ioServer->CreateStream(srcPath);
    Ptr<BinaryReader> reader = BinaryReader::Create();
    reader->SetStream(srcStream);
    if (!reader->Open())
    {
        n_printf("Не удалось открыть «%s» для чтения!\n", srcPath.AsCharPtr());
        return false;
    }
    Ptr<Stream> dstStream = ioServer->CreateStream(dstPath);
    Ptr<ModelWriter> modelWriter;
    if (this->binary)
    {
        modelWriter = BinaryModelWriter::Create();
    }
    else
    {
        modelWriter = XmlModelWriter::Create();
    }
    modelWriter->SetStream(dstStream);
    modelWriter->SetPlatform(this->platform);
    if (!modelWriter->Open())
    {
        n_printf("Не удалось открыть «%s» для записи!\n", dstPath.AsCharPtr());
        return false;
    }

    this->lastAnimatorName.Clear();
    this->animatedNodesToAnimator.Clear();
    this->animatorInformation.Clear();

    // выполнить фактическое преобразование
    String modelName = category;
    modelName.Append("/");
    modelName.Append(fileName);
    modelName.StripFileExtension();
    if (!this->PerformConversion(modelName, reader, modelWriter))
    {
        n_printf("Не удалось преобразовать: '%s'!\n", srcPath.AsCharPtr());
        return false;
    }

    // done
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2Converter::PerformConversion(const String& curModelName, const Ptr<BinaryReader>& reader, const Ptr<ModelWriter>& writer)
{
    // сначала убедитесь, что исходный файл на самом деле является файлом N2, и пропустите заголовок
    FourCC magic(reader->ReadUInt());
    if (magic != FourCC('NOB0'))
    {
        n_printf("Исходный файл «%s» не является двоичным файлом Nebula2!\n", this->srcPath.AsCharPtr());
        return false;
    }
    
    // пропустить данные заголовка источника
    String headerData = reader->ReadString();

    // проверьте, является ли файл сценой графического интерфейса
    if (this->IsGuiScene(reader))
    {
        // конвертировать в XML-файл пользовательского интерфейса
        this->ConvertGuiScene(reader, writer);
    }
    else
    {  
        // рекурсивно читать узлы
        writer->BeginModel("Models::Model", FourCC('MODL'), curModelName);
        if (!this->ParseNodes(reader, writer))
        {
            n_printf("Ошибка преобразования в RecurseReadNodes() ('%s')!\n", this->srcPath.AsCharPtr());
            return false;
        }
        writer->EndModel();
        n_assert(this->nodeTypeStack.IsEmpty());      
    }
    
    // все сделано
    return true;
}

//------------------------------------------------------------------------------
/**
*/
bool
N2Converter::ParseNodes(const Ptr<BinaryReader>& reader, const Ptr<ModelWriter>& writer)
{
    const FourCC newFourCC('_new');
    const FourCC selFourCC('_sel');
    FourCC curFourCC;
    while (!reader->Eof())
    {
        curFourCC = reader->ReadUInt();   
        if (curFourCC == newFourCC)
        {
            // сохранить информацию об узле с последнего узла
            this->FillAnimatorInformation();

            // обнаружен новый узел
            String objClass = reader->ReadString();
            String objName = reader->ReadString();
            this->SetAnimatorPath(objName, objClass);
            this->BeginNode(objClass, objName, writer);
        }
        else if (curFourCC == selFourCC)
        {
            // пропустить относительный путь '..'
            String selPath = reader->ReadString();
            // стереть последний сегмент пути
            if (path.Size() > 0)
            {
                this->path.EraseIndex(path.Size()-1);     
            }            
            this->EndNode(writer);
        }
        else
        {
            // прочитать текущий тег данных
            this->ReadDataTag(curFourCC, reader, writer);
        }
    }
    if (/*!this->animatorInformation.IsEmpty()
        && */!this->animatedNodesToAnimator.IsEmpty())
    {
        // установить последний узел аниматора
        this->FillAnimatorInformation();

        // написать дополнительный узел аниматора
        this->WriteAnimatorNode(writer);

        writer->EndModelNode();
    }

    // done
    return true;
}

//------------------------------------------------------------------------------
/**
*/  
void 
N2Converter::WriteAnimatorNode(const Ptr<ModelWriter>& writer)
{
    IndexT i;    
    writer->BeginModelNode("Model::AnimatorNode", FourCC('MANI'), "animator");
    for (i = 0; i < this->animatorInformation.Size(); i++)
    {
        AnimatorInfo& animInfo = this->animatorInformation.ValueAtIndex(i);

        // ловить ошибки
        if (((animInfo.animatorNode == IntAnimator) ||
            (animInfo.animatorNode == FloatAnimator) ||
            (animInfo.animatorNode == Float4Animator)) &&
            (animInfo.paramName.IsEmpty()))
        {
            continue;
        }

        if (this->animatedNodesToAnimator.Contains(this->animatorInformation.KeyAtIndex(i)))
        {
            writer->BeginTag("BeginAnimSection", FourCC('BASE'));
            writer->WriteInt((int)animInfo.animatorNode);
            
            const Array<String>& pathToNodes = this->animatedNodesToAnimator[this->animatorInformation.KeyAtIndex(i)];
            IndexT pathIdx;
            for (pathIdx = 0; pathIdx < pathToNodes.Size(); pathIdx++)
            {
                writer->BeginTag("AnimatedNodes", FourCC('ANNO'));
                writer->WriteString(pathToNodes[pathIdx]);
                writer->EndTag();
            }

            if (!animInfo.animation.IsEmpty())
            {
                writer->BeginTag("AnimationName", FourCC('SANI'));
                writer->WriteString(animInfo.animation);
                writer->EndTag();
            }

            if (!animInfo.loopType.IsEmpty())
            {
                writer->BeginTag("LoopType", FourCC('SLPT'));
                writer->WriteString(animInfo.loopType);
                writer->EndTag();
            }

            if (!animInfo.paramName.IsEmpty())
            {
                writer->BeginTag("ParamName", FourCC('SPNM'));
                writer->WriteString(animInfo.paramName);
                writer->EndTag();
            }

            if (!animInfo.key.IsEmpty())
            {
                writer->BeginTag("Key", FourCC('ADDK'));
                //format
                switch(animInfo.key[0].value.GetType())
                {
                case Variant::Float:
                    writer->WriteString("Float");
                    break;
                case Variant::Float4:
                    writer->WriteString("Float4");
                    break;
                case Variant::Int:
                    writer->WriteString("Int");
                    break;
                }
                //size
                writer->WriteInt((int)animInfo.key.Size());
                for(IndexT j = 0; j < animInfo.key.Size(); j++)
                {
                    writer->WriteFloat(animInfo.key[j].time);
                    switch (animInfo.key[j].value.GetType())
                    {
                    case Variant::Float:
                        writer->WriteFloat(animInfo.key[j].value.GetFloat());
                        break;
                    case Variant::Float4:
                        writer->WriteFloat4(animInfo.key[j].value.GetFloat4());
                        break;
                    case Variant::Int:
                        writer->WriteInt(animInfo.key[j].value.GetInt());
                        break;
                    }
                }

                writer->EndTag();
            }

            if (!animInfo.vectorName.IsEmpty())
            {
                writer->BeginTag("VectorName", FourCC('SVCN'));
                writer->WriteString(animInfo.vectorName);
                writer->EndTag();
            }

            if (animInfo.animationGroup > -1)
            {
                writer->BeginTag("AnimationGroup", FourCC('SAGR'));
                writer->WriteInt(animInfo.animationGroup);
                writer->EndTag();
            }

            if (!animInfo.posKey.IsEmpty())
            {
                writer->BeginTag("PositionKey", FourCC('ADPK'));
                if (animInfo.animatorNode == 5)  //uvAnimator
                {
                    writer->WriteInt((int)animInfo.posKey.Size());
                    for (IndexT j = 0; j < animInfo.posKey.Size(); j++)
                    {
                        writer->WriteInt(animInfo.layer[j]);
                        writer->WriteFloat(animInfo.posKey[j].time);
                        writer->WriteFloat4(animInfo.posKey[j].value.GetFloat4());
                    }
                }
                else    //transformanimator
                {
                    writer->WriteInt((int)animInfo.posKey.Size());
                    for(IndexT j = 0; j < animInfo.posKey.Size(); j++)
                    {
                        writer->WriteFloat(animInfo.posKey[j].time);
                        writer->WriteFloat4(animInfo.posKey[j].value.GetFloat4());
                    }
                }
                writer->EndTag();
            }

            if (!animInfo.eulerKey.IsEmpty())
            {
                writer->BeginTag("EulerKey", FourCC('ADEK'));
                if (animInfo.animatorNode == 5)
                {
                    writer->WriteInt((int)animInfo.eulerKey.Size());
                    for (IndexT j = 0; j < animInfo.eulerKey.Size(); j++)
                    {
                        writer->WriteInt(animInfo.layer[j]);
                        writer->WriteFloat(animInfo.eulerKey[j].time);
                        writer->WriteFloat4(animInfo.eulerKey[j].value.GetFloat4());
                    }
                }
                else 
                {
                    writer->WriteInt((int)animInfo.eulerKey.Size());
                    for(IndexT j = 0; j < animInfo.eulerKey.Size(); j++)
                    {
                        writer->WriteFloat(animInfo.eulerKey[j].time);
                        writer->WriteFloat4(animInfo.eulerKey[j].value.GetFloat4());
                    }
                }
                writer->EndTag();
            }

            if (!animInfo.scaleKey.IsEmpty())
            {
                writer->BeginTag("ScaleKey", FourCC('ADSK'));
                if (animInfo.animatorNode == 5)
                {
                    writer->WriteInt((int)animInfo.scaleKey.Size());
                    for (IndexT j = 0; j < animInfo.scaleKey.Size(); j++)
                    {
                        writer->WriteInt(animInfo.layer[j]);
                        writer->WriteFloat(animInfo.scaleKey[j].time);
                        writer->WriteFloat4(animInfo.scaleKey[j].value.GetFloat4());
                    }
                }
                else 
                {
                    writer->WriteInt((int)animInfo.scaleKey.Size());
                    for(IndexT j = 0; j < animInfo.scaleKey.Size(); j++)
                    {
                        writer->WriteFloat(animInfo.scaleKey[j].time);
                        writer->WriteFloat4(animInfo.scaleKey[j].value.GetFloat4());
                    }
                }
                writer->EndTag();
            }              
            writer->EndTag();   
        }
    }
    writer->EndModelNode();
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::BeginNode(const String& objClass, const String& objName, const Ptr<ModelWriter>& writer)
{
    // проверить известные классы
    if (objClass == "ntransformnode")
    {
        this->nodeType = TransformNode;
        this->animNodeType = InvalidAnimatorNodeType;
        writer->BeginModelNode("Models::TransformNode", FourCC('TRFN'), objName);
    }
    else if (objClass == "nshapenode")
    {
        this->nodeType = ShapeNode;
        this->animNodeType = InvalidAnimatorNodeType;
        writer->BeginModelNode("Models::ShapeNode", FourCC('SPND'), objName);
    }    
    else if (objClass == "nmultilayerednode")
    {
        this->nodeType = ShapeNode;
        this->animNodeType = InvalidAnimatorNodeType;
        writer->BeginModelNode("Models::ShapeNode", FourCC('SPND'), objName);
    }
    else if (objClass == "ncharacter3node")
    {
        this->nodeType = Character3Node;
        this->animNodeType = InvalidAnimatorNodeType;
        writer->BeginModelNode("Characters::CharacterNode", FourCC('CHRN'), objName);
        this->ConvertSkinLists(writer);
    }
    else if (objClass == "ncharacter3skinshapenode")
    {
        this->nodeType = Character3SkinShapeNode;
        this->animNodeType = InvalidAnimatorNodeType;
        writer->BeginModelNode("Characters::CharacterSkinNode", FourCC('CHSN'), objName);
    }
    else if (objClass == "nintanimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = IntAnimator;
        this->lastAnimatorName = this->ConvertRelNodePathToAbsolute(objName, true);
    }
    else if (objClass == "nfloatanimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = FloatAnimator;
        this->lastAnimatorName = this->ConvertRelNodePathToAbsolute(objName, true);
    }
    else if (objClass == "nvectoranimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = Float4Animator;
        this->lastAnimatorName = this->ConvertRelNodePathToAbsolute(objName, true);
    }
    else if (objClass == "ntransformanimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = TransformAnimator;
        this->lastAnimatorName = this->ConvertRelNodePathToAbsolute(objName, true);
    }
    else if (objClass == "ntransformcurveanimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = TransformCurveAnimator;
        this->lastAnimatorName = this->ConvertRelNodePathToAbsolute(objName, true);
    }
    else if (objClass == "nuvanimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = UvAnimator;
        this->lastAnimatorName = this->ConvertRelNodePathToAbsolute(objName, true);
    }
    else if (objClass == "ncharacter3skinanimator")
    {
        this->nodeType = EmbeddedNodeType;
        this->animNodeType = InvalidAnimatorNodeType;
    }
    else if (objClass == "nparticleshapenode2")
    {
        this->nodeType = ParticleSystemNode;
        writer->BeginModelNode("Particles::ParticleSystemNode", FourCC('PSND'), objName);
    }
    else if (objClass == "nlodnode")
    {
        // используйте обычный Transformnode для nlodnode
        this->nodeType = TransformNode;
        this->animNodeType = InvalidAnimatorNodeType;
        writer->BeginModelNode("Models::TransformNode", FourCC('TRFN'), objName);
    }
    else
    {
        this->nodeType = InvalidNodeType;
        this->animNodeType = InvalidAnimatorNodeType;
    }
    this->nodeTypeStack.Push(this->nodeType);
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::EndNode(const Ptr<ModelWriter>& writer)
{
    NodeType nodeType = this->nodeTypeStack.Pop();
    if ((nodeType != InvalidNodeType) && (nodeType != EmbeddedNodeType))
    {
        if (!this->nodeTypeStack.IsEmpty() || this->animatedNodesToAnimator.IsEmpty())
        {
            writer->EndModelNode();
        }
    }
    this->nodeType = InvalidNodeType;
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::ReadDataTag(FourCC fourCC, const Ptr<BinaryReader>& reader, const Ptr<ModelWriter>& writer)
{
    ushort length = reader->ReadUShort();
    if (InvalidNodeType == this->nodeType)
    {
        // if current node type is unknown, skip the tag's data block 
        reader->GetStream()->Seek(length, Stream::Current);
    }
    else
    {
        if ((FourCC('SLCB') == fourCC) && (this->nodeType != EmbeddedNodeType))
        {
            // setlocalbox
            float x, y, z;
            writer->BeginTag("LocalBox", FourCC('LBOX'));
            x = reader->ReadFloat();
            y = reader->ReadFloat();
            z = reader->ReadFloat();
            writer->WriteFloat4(point(x, y, z));
            x = reader->ReadFloat();
            y = reader->ReadFloat();
            z = reader->ReadFloat();
            writer->WriteFloat4(vector(x, y, z));
            writer->EndTag();
        }
        else if (FourCC('SPOS') == fourCC)
        {
            // setposition
            writer->BeginTag("Position", FourCC('POSI'));
            float x = reader->ReadFloat();
            float y = reader->ReadFloat();
            float z = reader->ReadFloat();
            writer->WriteFloat4(point(x, y, z));
            writer->EndTag();
        }
        else if (FourCC('SQUT') == fourCC)
        {
            // setquat
            writer->BeginTag("Rotation", FourCC('ROTN'));
            writer->WriteFloat4(reader->ReadFloat4());
            writer->EndTag();
        }
        else if (FourCC('SSCL') == fourCC)
        {
            // setscale
            writer->BeginTag("Scale", FourCC('SCAL'));
            float x = reader->ReadFloat();
            float y = reader->ReadFloat();
            float z = reader->ReadFloat();
            writer->WriteFloat4(vector(x, y, z));
            writer->EndTag();
        }
        else if (FourCC('SRTP') == fourCC)
        {
            // setrotatepivot
            writer->BeginTag("RotatePivot", FourCC('RPIV'));
            float x = reader->ReadFloat();
            float y = reader->ReadFloat();
            float z = reader->ReadFloat();
            writer->WriteFloat4(point(x, y, z));
            writer->EndTag();
        }
        else if (FourCC('SSCP') == fourCC)
        {
            // setscalepivot
            writer->BeginTag("ScalePivot", FourCC('SPIV'));
            float x = reader->ReadFloat();
            float y = reader->ReadFloat();
            float z = reader->ReadFloat();
            writer->WriteFloat4(point(x, y, z));
            writer->EndTag();
        }
        else if (FourCC('SVSP') == fourCC)
        {
            // флаг setviewspace
            writer->BeginTag("RelativeToViewSpace", FourCC('SVSP')); 
            writer->WriteBool(reader->ReadBool());
            writer->EndTag();
        }
        else if (FourCC('SLKV') == fourCC)
        {
            // флаг setlockviewer
            writer->BeginTag("LockViewer", FourCC('SLKV')); 
            writer->WriteBool(reader->ReadBool());
            writer->EndTag();
        }
        else if (FourCC('SMID') == fourCC)
        {
            // флаг setmindistance
            writer->BeginTag("MinDistance", FourCC('SMID')); 
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SMAD') == fourCC)
        {
            // флаг setmaxdistance
            writer->BeginTag("MaxDistance", FourCC('SMAD')); 
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('STXT') == fourCC)
        {
            // settexture
            writer->BeginTag("ShaderTexture", FourCC('STXT'));
            writer->WriteString(this->ConvertShaderSemantic(reader->ReadString()));

            String resId = this->SubstituteResourceAssigns(reader->ReadString());
            this->AddUsedResource(resId);
            resId.StripFileExtension();
            writer->WriteString(resId);
            writer->EndTag();
        }
        else if (FourCC('SINT') == fourCC)
        {
            // setint
            writer->BeginTag("ShaderInt", FourCC('SINT'));
            writer->WriteString(this->ConvertShaderSemantic(reader->ReadString()));
            writer->WriteInt(reader->ReadInt());
            writer->EndTag();
        }
        else if (FourCC('SFLT') == fourCC)
        {
            // setfloat
            writer->BeginTag("ShaderFloat", FourCC('SFLT'));
            writer->WriteString(this->ConvertShaderSemantic(reader->ReadString()));
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SVEC') == fourCC)
        {
            // setvector
            writer->BeginTag("ShaderVector", FourCC('SVEC'));
            writer->WriteString(this->ConvertShaderSemantic(reader->ReadString()));
            writer->WriteFloat4(reader->ReadFloat4());
            writer->EndTag();
        }
        else if (FourCC('SSHD') == fourCC)
        {
            // setshader
            String shaderName = reader->ReadString();
            writer->BeginTag("Shader", FourCC('SHDR'));
            writer->WriteString(this->ConvertShaderName(shaderName));
            writer->EndTag();

            // сопоставить имя шейдера с ModelNodeType
            writer->BeginTag("ModelNodeType", FourCC('MNTP'));
            writer->WriteString(this->MapShaderNameToModelNodeType(shaderName));
            writer->EndTag();
        }
        else if (FourCC('SMSH') == fourCC)
        {
            // setmesh
            writer->BeginTag("Mesh", FourCC('MESH'));
            String resId = this->SubstituteResourceAssigns(reader->ReadString());
            this->AddUsedResource(resId);
            writer->WriteString(resId);
            writer->EndTag();
        }
        else if (FourCC('SGRI') == fourCC)
        {
            // setgroupindex
            writer->BeginTag("PrimitiveGroupIndex", FourCC('PGRI'));
            writer->WriteInt(reader->ReadInt());
            writer->EndTag();
        }
        else if (FourCC('SANM') == fourCC)
        {
            // setanim
            writer->BeginTag("Animation", FourCC('ANIM'));
            String resId = this->SubstituteResourceAssigns(reader->ReadString());
            this->AddUsedResource(resId);
            writer->WriteString(this->ConvertAnimationName(resId));
            writer->EndTag();
        }
        else if (FourCC('BJNT') == fourCC)
        {
            // beginjoints
            writer->BeginTag("NumJoints", 'NJNT');
            writer->WriteInt(reader->ReadInt());
            writer->EndTag();
        }
        else if (FourCC('SJNT') == fourCC)
        {
            // setjoint
            writer->BeginTag("Joint", 'JONT');
            int jointIndex = reader->ReadInt();
            int parentJointIndex = reader->ReadInt();
            point poseTranslation;
            poseTranslation.x() = reader->ReadFloat();
            poseTranslation.y() = reader->ReadFloat();
            poseTranslation.z() = reader->ReadFloat();
            float4 poseRotation = reader->ReadFloat4();
            vector poseScale;
            poseScale.x() = reader->ReadFloat();
            poseScale.y() = reader->ReadFloat();
            poseScale.z() = reader->ReadFloat();
            String name = reader->ReadString();
            writer->WriteInt(jointIndex);
            writer->WriteInt(parentJointIndex);
            writer->WriteFloat4(poseTranslation);
            writer->WriteFloat4(poseRotation);
            writer->WriteFloat4(poseScale);
            writer->WriteString(name);
            writer->EndTag();
        }
        else if (FourCC('SVRT') == fourCC)
        {
            // setvariations
            writer->BeginTag("Variations", 'VART');
            String resId = this->SubstituteResourceAssigns(reader->ReadString());
            this->AddUsedResource(resId);
            writer->WriteString(this->ConvertAnimationName(resId));
            writer->EndTag();
        }
        else if (FourCC('BGFR') == fourCC)
        {
            // setnumfragments
            int numFragments = reader->ReadInt();
            writer->BeginTag("NumSkinFragments", 'NSKF');
            writer->WriteInt(numFragments);
            writer->EndTag();
            this->skinFragmentPrimGroupIndices.SetSize(numFragments);
            this->skinFragmentJointPalettes.SetSize(numFragments);
        }
        else if (FourCC('SFGI') == fourCC)
        {
            IndexT fragIndex = reader->ReadInt();
            this->skinFragmentPrimGroupIndices[fragIndex] = reader->ReadInt();
        }
        else if (FourCC('BGJP') == fourCC)
        {
            // beginjointpalette            
            int fragIndex = reader->ReadInt();
            int skinFragmentNumJoints = reader->ReadInt();            
            this->skinFragmentJointPalettes[fragIndex].SetSize(skinFragmentNumJoints);
        }
        else if (FourCC('SJID') == fourCC)
        {
            // setjointindices
            // пропустить ненужные данные
            int fragIndex = reader->ReadInt();
            int paletteStartIndex = reader->ReadInt();

            // добавить до 8 индексов суставов
            IndexT i;
            IndexT curIndex = paletteStartIndex;
            for (i = 0; i < 8; i++)
            {
                int jointIndex = reader->ReadInt();
                if (curIndex < this->skinFragmentJointPalettes[fragIndex].Size())
                {
                    this->skinFragmentJointPalettes[fragIndex][curIndex++] = jointIndex;
                }
            }
        }
        else if (FourCC('EDFR') == fourCC)
        {
            // endfragment
            IndexT i;
            for (i = 0; i < this->skinFragmentPrimGroupIndices.Size(); i++)
            {
                writer->BeginTag("SkinFragment", 'SFRG');
                writer->WriteInt(this->skinFragmentPrimGroupIndices[i]);
                writer->WriteIntArray(this->skinFragmentJointPalettes[i].AsArray());
                writer->EndTag();
            }
        }        
        else if (FourCC('SCVA') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleEmissionFrequency", 'EFRQ', reader, writer);
        }
        else if (FourCC('SCVB') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleLifeTime", 'PLFT', reader, writer);
        }
        else if (FourCC('SCVD') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleSpreadMin", 'PSMN', reader, writer);
        }
        else if (FourCC('SCVE') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleSpreadMax", 'PSMX', reader, writer);
        }
        else if (FourCC('SCVF') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleStartVelocity", 'PSVL', reader, writer);
        }
        else if (FourCC('SCVH') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleRotationVelocity", 'PRVL', reader, writer);
        }
        else if (FourCC('SCVJ') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleSize", 'PSZE', reader, writer);
        }
        else if (FourCC('SCVL') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleMass", 'PMSS', reader, writer);
        }
        else if (FourCC('STMM') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleTimeManipulator", 'PTMN', reader, writer);
        }
        else if (FourCC('SCVN') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleVelocityFactor", 'PVLF', reader, writer);
        }
        else if (FourCC('SCVQ') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleAirResistance", 'PAIR', reader, writer);
        }
        else if (FourCC('SCVC') == fourCC)
        {
            this->WriteParticleRGBEnvelopeCurve("ParticleRed", 'PRED', "ParticleGreen", 'PGRN', "ParticleBlue", 'PBLU', reader, writer);
        }
        else if (FourCC('SCVM') == fourCC)
        {
            this->WriteParticleEnvelopeCurve("ParticleAlpha", 'PALP', reader, writer);
        }
        else if (FourCC('SEMD') == fourCC)
        {
            writer->BeginTag("ParticleEmissionDuration", 'PEDU');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SLOP') == fourCC)
        {
            writer->BeginTag("ParticleLoopEmission", 'PLPE');
            writer->WriteInt(reader->ReadUChar());
            writer->EndTag();
        }
        else if (FourCC('SACD') == fourCC)
        {
            writer->BeginTag("ParticleActivityDistance", 'PACD');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SROF') == fourCC)
        {
            writer->BeginTag("ParticleRenderOldestFirst", 'PROF');
            writer->WriteInt(reader->ReadUChar());
            writer->EndTag();
        }
        else if (FourCC('SBBO') == fourCC)
        {
            writer->BeginTag("ParticleBillboardOrientation", 'PBBO');
            writer->WriteInt(reader->ReadUChar());
            writer->EndTag();
        }
        else if (FourCC('SRMN') == fourCC)
        {
            writer->BeginTag("ParticleRotationMin", 'PRMN');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SRMX') == fourCC)
        {
            writer->BeginTag("ParticleRotationMax", 'PRMX');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SGRV') == fourCC)
        {
            writer->BeginTag("ParticleGravity", 'PGRV');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SPST') == fourCC)
        {
            writer->BeginTag("ParticleStretch", 'PSTC');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('STTX') == fourCC)
        {
            writer->BeginTag("ParticleTextureTile", 'PTTX');
            writer->WriteFloat((float)reader->ReadInt());
            writer->EndTag();
        }
        else if (FourCC('SSTS') == fourCC)
        {
            writer->BeginTag("ParticleStretchToStart", 'PSTS');
            writer->WriteInt(reader->ReadUChar());
            writer->EndTag();
        }
        else if (FourCC('SCVR') == fourCC)
        {
            writer->BeginTag("ParticleVelocityRandomize", 'PVRM');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SCVS') == fourCC)
        {
            writer->BeginTag("ParticleRotationRandomize", 'PRRM');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SCVT') == fourCC)
        {
            writer->BeginTag("ParticleSizeRandomize", 'PSRM');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SPCT') == fourCC)
        {
            writer->BeginTag("ParticlePrecalcTime", 'PPCT');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('SCVU') == fourCC)
        {
            writer->BeginTag("ParticleRandomRotDir", 'PRRD');
            writer->WriteInt(reader->ReadUChar());
            writer->EndTag();
        }
        else if (FourCC('SSDT') == fourCC)
        {
            writer->BeginTag("ParticleStretchDetail", 'PSDL');
            writer->WriteInt(reader->ReadInt());
            writer->EndTag();
        }
        else if (FourCC('SVAF') == fourCC)
        {
            writer->BeginTag("ParticleViewAngleFade", 'PVAF');
            writer->WriteInt(reader->ReadUChar());
            writer->EndTag();
        }
        else if (FourCC('STDL') == fourCC)
        {
            writer->BeginTag("ParticleStartDelay", 'PDEL');
            writer->WriteFloat(reader->ReadFloat());
            writer->EndTag();
        }
        else if (FourCC('STUS') == fourCC)
        {
            Array<Math::float4> multilayerStretch;
            Array<Math::float4> multilayerSpec;
            // FIXME: размер слоя фиксирован до 8 слоев.
            IndexT i;
            const SizeT numLayer = 8;
            float curStretch[numLayer];
            float curSpec[numLayer];
            for (i = 0; i < numLayer; i++)
            {
                n_assert(reader->ReadInt() == i);
                curStretch[i] = reader->ReadFloat();

                FourCC curFourCC = reader->ReadUInt(); 
                n_assert2(FourCC('SSPI') == curFourCC, "Чтение многоуровневого параметра: Неверная четверкаCC!");
                ushort length = reader->ReadUShort();

                n_assert(reader->ReadInt() == i);
                curSpec[i] = reader->ReadFloat();
                
                if (i < numLayer - 1)
                {
                    curFourCC = reader->ReadUInt(); 
                    n_assert(FourCC('STUS') == curFourCC);
                    length = reader->ReadUShort();
                }
            }  

            // group single stretch and spec values to float4 for better use as shadervariables
            const int float4Size = 4;
            SizeT numVars = numLayer / float4Size;
            for (i = 0; i < numVars; i++)
            {
                // stretch
                writer->BeginTag("MultilayerUVStretch", 'STUS');
                writer->WriteInt(i);
                writer->WriteFloat4(Math::float4(curStretch[i * float4Size],
                                    curStretch[i * float4Size + 1],
                                    curStretch[i * float4Size + 2],
                                    curStretch[i * float4Size + 3]));
                writer->EndTag();

                // specular
                writer->BeginTag("MultilayerSpecIntensity", 'SSPI');
                writer->WriteInt(i);
                writer->WriteFloat4(Math::float4(curSpec[i * float4Size],
                                    curSpec[i * float4Size + 1],
                                    curSpec[i * float4Size + 2],
                                    curSpec[i * float4Size + 3]));
                writer->EndTag();  
            }
        }

        else if (FourCC('ADDA') == fourCC)
        {
            String absPath = this->ConvertRelNodePathToAbsolute(reader->ReadString());
            IndexT index = this->animatedNodesToAnimator.FindIndex(absPath);

            if (index == -1)
            {
                // add new Key if Absolute Path not exists
                Array<String> nodePath;
                nodePath.Append(this->ConvertNodePathFromArrayToString(this->path));
                this->animatedNodesToAnimator.Add(absPath, nodePath);
            }
            else
            {
                // добавить путь к ключу, если ключ уже существует
                this->animatedNodesToAnimator[absPath].Append(this->ConvertNodePathFromArrayToString(this->path));
            }
        }
        else if (FourCC('SLPT') == fourCC)
        {
            // nebula3 не использует скин-аниматоров, поэтому FourCC используется только другими аниматорами.
            if (this->animNodeType >= IntAnimator && this->animNodeType <= UvAnimator)
            {
                this->animInfo.loopType = reader->ReadString();
            }
            else
            {                
                reader->GetStream()->Seek(length, Stream::Current);
            }
        }
        else if (FourCC('ADDK') == fourCC)
        {
            if (this->animNodeType == Float4Animator)
            {
                KeyValues val;
                val.time = reader->ReadFloat();
                val.value = reader->ReadFloat4();
                this->animInfo.key.Append(val);
            }
            else if (this->animNodeType == FloatAnimator)
            {
                KeyValues val;
                val.time = reader->ReadFloat();
                val.value = reader->ReadFloat();
                this->animInfo.key.Append(val);
            }
            else if (this->animNodeType == IntAnimator)
            {
                KeyValues val;
                val.time = reader->ReadFloat();
                val.value = reader->ReadInt();
                this->animInfo.key.Append(val);
            }
            else
            {                
                reader->GetStream()->Seek(length, Stream::Current);
            }
        }
        else if (FourCC('SPNM') == fourCC)
        {
            //ShaderParamName
            this->animInfo.paramName = reader->ReadString();
        }
        else if (FourCC('SVCN') == fourCC)
        {
            //VectorName
            this->animInfo.vectorName = reader->ReadString();
        }
        else if (FourCC('ADPK') == fourCC)
        {
            //PositionKey
            KeyValues val;
            if (this->animNodeType == UvAnimator)
            {
                this->animInfo.layer.Append(reader->ReadInt());
                val.time = (float)reader->ReadFloat();
                float u = reader->ReadFloat();
                float v = reader->ReadFloat();
                val.value.SetFloat4(Math::float4(u, v, 0, 1));
            }
            else //transformanimator
            {
                val.time = (float)reader->ReadFloat();
                float x = reader->ReadFloat();
                float y = reader->ReadFloat();
                float z = reader->ReadFloat();
                val.value.SetFloat4(Math::float4(x, y, z, 1));
            }
            
            this->animInfo.posKey.Append(val);
        }
        else if (FourCC('ADEK') == fourCC)
        {
            //EulerKey
            KeyValues val;
            if (this->animNodeType == UvAnimator)
            {
                this->animInfo.layer.Append(reader->ReadInt());
                val.time = (float)reader->ReadFloat();
                float u = n_deg2rad(reader->ReadFloat());
                float v = n_deg2rad(reader->ReadFloat());
                val.value.SetFloat4(Math::float4(u, v, 0, 1));
            }
            else //transformanimator
            {
                val.time = (float)reader->ReadFloat();
                float x = reader->ReadFloat();
                float y = reader->ReadFloat();
                float z = reader->ReadFloat();
                val.value.SetFloat4(Math::float4(x, y, z, 1));
            }
            this->animInfo.eulerKey.Append(val);
        }
        else if (FourCC('ADSK') == fourCC)
        {
            //ScaleKey
            KeyValues val;
            if (this->animNodeType == UvAnimator)
            {
                this->animInfo.layer.Append(reader->ReadInt());
                val.time = (float)reader->ReadFloat();
                float u = reader->ReadFloat();
                float v = reader->ReadFloat();
                val.value.SetFloat4(Math::float4(u, v, 1, 1));
            }
            else //transformanimator
            {
                val.time = (float)reader->ReadFloat();
                float x = reader->ReadFloat();
                float y = reader->ReadFloat();
                float z = reader->ReadFloat();
                val.value.SetFloat4(Math::float4(x, y, z, 1));
            }
            this->animInfo.scaleKey.Append(val);
        }
        else if (FourCC('SANI') == fourCC)
        {
            //animation
            String resId = this->SubstituteResourceAssigns(reader->ReadString());
            this->AddUsedResource(resId);
            this->animInfo.animation = this->ConvertAnimationName(resId);
        }
        else if (FourCC('SAGR') == fourCC)
        {
            //animationgroup
            this->animInfo.animationGroup = reader->ReadInt();
        }
        else if (FourCC('SSTA') == fourCC)
        {
            writer->BeginTag("StringAttr", 'SSTA');
            writer->WriteString(reader->ReadString());
            writer->WriteString(reader->ReadString());
            writer->EndTag();
        }
        else
        {
            // пропускать неизвестные теги данных
            reader->GetStream()->Seek(length, Stream::Current);
        }
    }
}

//------------------------------------------------------------------------------
/**
    Reads a particle system envelope curve data entry from the reader, 
    and writes it as a new data tag to the model writer.
*/
void
N2Converter::WriteParticleEnvelopeCurve(const String& tagName, FourCC fourCC, const Ptr<BinaryReader>& reader, const Ptr<ModelWriter>& writer)
{
    writer->BeginTag(tagName, fourCC);
    writer->WriteFloat(reader->ReadFloat());    // val0
    writer->WriteFloat(reader->ReadFloat());    // val1
    writer->WriteFloat(reader->ReadFloat());    // val2
    writer->WriteFloat(reader->ReadFloat());    // val3
    writer->WriteFloat(reader->ReadFloat());    // keyPos0
    writer->WriteFloat(reader->ReadFloat());    // keyPos1
    writer->WriteFloat(reader->ReadFloat());    // frequency
    writer->WriteFloat(reader->ReadFloat());    // amplitude
    writer->WriteInt(reader->ReadInt());        // modfunc
    writer->EndTag();
}

//------------------------------------------------------------------------------
/**
    Специальная версия WriteParticleEnvelopeCurve, которая читает
    Огибающую кривую RGB и записывает ее как 3 независимые огибающие кривые.
*/
void
N2Converter::WriteParticleRGBEnvelopeCurve(const String& redTagName, FourCC redFourCC,
                                           const String& greenTagName, FourCC greenFourCC,
                                           const String& blueTagName, FourCC blueFourCC,
                                           const Ptr<BinaryReader>& reader,
                                           const Ptr<ModelWriter>& writer)
{
    // прочитать исходные данные
    float red[4], green[4], blue[4];
    float keyPos0, keyPos1;
    float frequency, amplitude;
    int modFunc;
    IndexT i;
    for (i = 0; i < 4; i++)
    {
        red[i]   = reader->ReadFloat();
        green[i] = reader->ReadFloat();
        blue[i]  = reader->ReadFloat();
    }
    keyPos0 = reader->ReadFloat();
    keyPos1 = reader->ReadFloat();

    // ПРИМЕЧАНИЕ. Nebula2-RGB-EnvelopeCurves не имеет синусоидальной/косинусной модуляции!
    frequency = 1.0f;
    amplitude = 0.0f;
    modFunc = 0;

    // написать красную метку данных
    writer->BeginTag(redTagName, redFourCC);
    writer->WriteFloat(red[0]);
    writer->WriteFloat(red[1]);
    writer->WriteFloat(red[2]);
    writer->WriteFloat(red[3]);
    writer->WriteFloat(keyPos0);
    writer->WriteFloat(keyPos1);
    writer->WriteFloat(frequency);
    writer->WriteFloat(amplitude);
    writer->WriteInt(modFunc);
    writer->EndTag();

    // написать зеленый тег данных
    writer->BeginTag(greenTagName, greenFourCC);
    writer->WriteFloat(green[0]);
    writer->WriteFloat(green[1]);
    writer->WriteFloat(green[2]);
    writer->WriteFloat(green[3]);
    writer->WriteFloat(keyPos0);
    writer->WriteFloat(keyPos1);
    writer->WriteFloat(frequency);
    writer->WriteFloat(amplitude);
    writer->WriteInt(modFunc);
    writer->EndTag();

    // записать синий тег данных
    writer->BeginTag(blueTagName, blueFourCC);
    writer->WriteFloat(blue[0]);
    writer->WriteFloat(blue[1]);
    writer->WriteFloat(blue[2]);
    writer->WriteFloat(blue[3]);
    writer->WriteFloat(keyPos0);
    writer->WriteFloat(keyPos1);
    writer->WriteFloat(frequency);
    writer->WriteFloat(amplitude);
    writer->WriteInt(modFunc);
    writer->EndTag();
}


//------------------------------------------------------------------------------
/**
*/
String
N2Converter::MapShaderNameToModelNodeType(const String& shaderName)
{
    if ("simple_alpha" == shaderName
        || "particle2_unlit" == shaderName)
    {
        return "Alpha";
    } 
    else if ("particle2" == shaderName)
    {
        return "ParticleLit";
    }
    else if ("additive_alpha" == shaderName)
    {
        return "Additive";
    }
    else if (String::MatchPattern(shaderName, "*альфа*") ||
             "volumefog" == shaderName ||  
             "hair" == shaderName ||
             "skinned_alpha" == shaderName)
    {
        return "AlphaLit";
    }
    else if ("particle2_additive" == shaderName)
    {
        return "Additive";
    }
    else if ("static_atest" == shaderName)
    {
        return "AlphaTest";
    }
    else if ("skybox" == shaderName)
    {
        return "Background";
    }
    else if ("gui3d" == shaderName)
    {
        return "UnlitAlpha";
    }
    else if ("qwiiz_solid_unlit" == shaderName)
    {
        return "UnlitSolid";
    }
    else if ("qwiiz_translucent" == shaderName)
    {
        return "Translucent";
    }
    else
    {
        return "Solid";
    }
}

//------------------------------------------------------------------------------
/**
*/
String
N2Converter::ConvertShaderName(const String& shdName)
{
    if ("lightmapped" == shdName
        || "lightmapped2" == shdName
        || "lightmapped_alpha" == shdName)
    {
        return "shd:lightmapped";
    }
    else if ("lightmapped2_skinned" == shdName)
    {
        return "shd:lightmapped2_skinned";
    }
    else if ("lightmapped_environment" == shdName)
    {
        return "shd:lightmapped_environment";
    }
    else if ("skybox" == shdName)
    {
        return "shd:skybox";
    }
    else if ("multilayered" == shdName 
        ||"multilayeredbump" == shdName)
    {
        return "shd:multilayer";
    }
    else if ("volumefog" == shdName)
    {
        return "shd:volumefog";
    }
    else if ("alpha_uvanimation" == shdName)
    {
        return "shd:uvanimated";
    }
    else if ("particle2" == shdName)
    {
        return "shd:particle";
    }
    else if ("particle2_additive" == shdName)
    {
        return "shd:particle";
    }
    else if ("particle2_unlit" == shdName)
    {
        return "shd:particle";
    }
    else if ("simple_alpha" == shdName)
    {
        return "shd:unlit";
    }
    else if ("environment_skinned" == shdName
           || "environment" == shdName
           || "environment_alpha" == shdName)
    {
        return "shd:environment";
    }
    else if ("gui3d" == shdName)
    {
        return "shd:unlit";
    }
    else if ("qwiiz_lm_alpha" == shdName)
    {
        return "shd:qwiiz_lm_alpha";
    }
    else if ("qwiiz_lm_env_alpha" == shdName)
    {
        return "shd:qwiiz_lm_env_alpha";
    }
    else if ("qwiiz_solid_unlit" == shdName)
    {
        return "shd:qwiiz_solid_unlit";
    }
    else if ("qwiiz_toon_specularmap" == shdName)
    {
        return "shd:qwiiz_toon_specularmap";
    }
    else if ("qwiiz_uvanimation_lightmapped" == shdName)
    {
        return "shd:qwiiz_uvanimated_lightmapped";
    }
    else
    {
        // отобразить все остальное в статике
        return "shd:static";
    }
}

//------------------------------------------------------------------------------
/**
    При необходимости преобразуйте семантическое имя шейдера (на PS3
    семантика — все в верхнем регистре).
*/
String
N2Converter::ConvertShaderSemantic(const String& sem) const
{
   return sem;   
}

//------------------------------------------------------------------------------
/**
    Преобразовать идентификатор ресурса анимации (просто меняет расширение файла).
*/
String
N2Converter::ConvertAnimationName(const String& anim) const
{
    String res = anim;
    res.StripFileExtension();
    res.Append(".nax3");
    return res;
}

//------------------------------------------------------------------------------
/**
    Выполните проверку времени файла, чтобы решить, должна ли текстура быть
    преобразован.
*/
bool
N2Converter::NeedsConversion(const String& srcPath, const String& dstPath)
{
    // проверка времени файла переопределена?
    if (this->force)
    {
        return true;
    }

    // if this is a character, we would need to check skin lists as well,
    // вместо этого мы просто предполагаем, что необходимо преобразование
    if (String::MatchPattern(srcPath, "*персонажи*"))
    {
        return true;
    }

    // в противном случае проверьте время файлов src и dst.
    IoServer* ioServer = IoServer::Instance();
    if (ioServer->FileExists(dstPath))
    {
        FileTime srcFileTime = ioServer->GetFileWriteTime(srcPath);
        FileTime dstFileTime = ioServer->GetFileWriteTime(dstPath);
        if (dstFileTime > srcFileTime)
        {
            // файл dst новее, чем файл src, конвертировать не нужно
            return false;
        }
    }

    // провал: файл dst не существует или он старше файла src
    return true;
}

//------------------------------------------------------------------------------
/**
    Loads the skin lists for the current character and writes them
    into the n3 file. This method is called by EndNode() when
    a Character3Node is finished.
*/
bool
N2Converter::ConvertSkinLists(const Ptr<ModelWriter>& writer)
{
    IoServer* ioServer = IoServer::Instance();
    
    // создать каталог списка скинов из текущего имени файла
    String skinListDir = this->srcPath;
    skinListDir.StripFileExtension();
    skinListDir.Append("/skinlists");
    if (!ioServer->DirectoryExists(skinListDir))
    {
        return false;
    }

    // получить файлы скинов для этого персонажа
    Array<String> skinLists = ioServer->ListFiles(skinListDir, "*.xml");
    if (skinLists.Size() == 0)
    {
        return false;
    }

    // записать количество списков скинов в файл N3
    writer->BeginTag("NumSkinLists", 'NSKL');
    writer->WriteInt(skinLists.Size());
    writer->EndTag();

    // Разобрать XML-файлы списка скинов
    IndexT skinListIndex;
    for (skinListIndex = 0; skinListIndex < skinLists.Size(); skinListIndex++)
    {   
        String skinListName = skinLists[skinListIndex];
        skinListName.StripFileExtension();
        Array<String> skins;
        
        // прочитать список скинов в программе чтения XML
        String skinListPath = skinListDir + "/" + skinLists[skinListIndex];
        Ptr<Stream> stream = ioServer->CreateStream(skinListPath);
        Ptr<XmlReader> xmlReader = XmlReader::Create();
        xmlReader->SetStream(stream);
        if (xmlReader->Open())
        {
            xmlReader->SetToNode("/skins");
            if (xmlReader->SetToFirstChild()) do
            {
                xmlReader->SetToFirstChild();
                xmlReader->SetToFirstChild();
                skins.Append(xmlReader->GetCurrentNodeName());
                xmlReader->SetToParent();
                xmlReader->SetToParent();
            }
            while (xmlReader->SetToNextChild());
            xmlReader->Close();
        }

        // записать список скинов в файл n3
        writer->BeginTag("SkinList", 'SKNL');
        writer->WriteString(skinListName);
        writer->WriteInt(skins.Size());
        IndexT skinIndex;
        for (skinIndex = 0; skinIndex < skins.Size(); skinIndex++)
        {
            writer->WriteString(skins[skinIndex]);
        }
        writer->EndTag();
    }
    return true;
}

//------------------------------------------------------------------------------
/**
*/
N2Converter::AnimatorNodeType
N2Converter::FromStringToAnimatorNode(String animNode)
{
    if (animNode == "ntransformcurveanimator")      return TransformCurveAnimator;
    else if (animNode == "ntransformanimator")      return TransformAnimator;
    else if (animNode == "nfloatanimator")          return FloatAnimator;
    else if (animNode == "nintanimator")            return IntAnimator;
    else if (animNode == "nvectoranimator")         return Float4Animator;
    else if (animNode == "nuvanimator")             return UvAnimator;
    else return InvalidAnimatorNodeType;
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::FillAnimatorInformation()
{
    if ((!this->lastAnimatorName.IsEmpty()) && (this->animInfo.animatorNode != InvalidAnimatorNodeType))
    {
        this->animatorInformation.Add(this->lastAnimatorName, this->animInfo);
        this->lastAnimatorName.Clear();

        //Очистка BufferVariable для дальнейшего использования.
        this->animInfo.animation.Clear();
        this->animInfo.key.Clear();
        this->animInfo.loopType.Clear();
        this->animInfo.paramName.Clear();
        this->animInfo.animatorNode = InvalidAnimatorNodeType;
        this->animInfo.animationGroup = -1;
        this->animInfo.vectorName.Clear();
        this->animInfo.posKey.Clear();
        this->animInfo.eulerKey.Clear();
        this->animInfo.scaleKey.Clear();
        this->animInfo.layer.Clear();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
N2Converter::SetAnimatorPath(String objName, String objClass)
{
    // построить путь
    if(path.Size() > 0)
        this->path.Append("/"+objName);
    else
        this->path.Append(objName);

    //сохраните объектный класс для дальнейшего использования
    this->animInfo.animatorNode = this->FromStringToAnimatorNode(objClass);
}

//------------------------------------------------------------------------------
/**
*/
String
N2Converter::ConvertNodePathFromArrayToString(Array<String> stringArray)
{
    IndexT tokenIdx;
    String returnValue;

    for (tokenIdx = 0; tokenIdx < stringArray.Size(); tokenIdx++)
    {
        returnValue.Append(stringArray[tokenIdx]);
    }

    return returnValue;
}

//------------------------------------------------------------------------------
/**
    Замените старые назначения Nebula2 на назначения Nebula3.
    (например, текстуры: -> tex:)
*/
String
N2Converter::SubstituteResourceAssigns(const String& str)
{
    String result = str;
    result.SubstituteString("textures:", "tex:");
    result.SubstituteString("meshes:", "msh:");
    result.SubstituteString("anims:", "ani:");
    return result;
}

//------------------------------------------------------------------------------
/**
    Преобразует относительный AnimatorPath в абсолютный AnimatorPath.
*/
String
N2Converter::ConvertRelNodePathToAbsolute(String relPath, bool pathToAnimatorNode)
{
    Array<String> tokenized = relPath.Tokenize("/");
    IndexT tokenIdx;
    Array<String> nodePath = this->path;

    for(tokenIdx = tokenized.Size()-1; tokenIdx > -1; tokenIdx--)
    {
        if (tokenized[tokenIdx] == "..")
        {
            //Удалить последний «..» и последний сегмент nodePath.
            nodePath.EraseIndex(nodePath.Size()-1);
            tokenized.EraseIndex(tokenIdx);
        }
    }
    String fragmentRelativePath = this->ConvertNodePathFromArrayToString(tokenized);
    String fragmentNodePath = this->ConvertNodePathFromArrayToString(nodePath);

    if (!pathToAnimatorNode)
    {
        // return Absolute Path
        if (fragmentNodePath.IsEmpty())
            return fragmentRelativePath;
        else
        {
            return fragmentNodePath + "/" + fragmentRelativePath;
        }
    }
    else
    {
        return fragmentNodePath;
    }
}

//------------------------------------------------------------------------------
/**
*/
bool 
N2Converter::IsGuiScene(const Ptr<BinaryReader>& reader)
{
    Stream::Position oldPos = reader->GetStream()->GetPosition();

    bool isGuiScene = false;
    const FourCC newFourCC('_new');
    const FourCC selFourCC('_sel');
    FourCC curFourCC;
    while (!reader->Eof() && !isGuiScene)
    {
        curFourCC = reader->ReadUInt();   
        if (curFourCC == newFourCC)
        {
            // новый узел обнаружил пропуск строк
            String objClass = reader->ReadString();
            String objName = reader->ReadString();
        }
        else if (curFourCC == selFourCC)
        {
            // пропустить относительный путь '..'
            String selPath = reader->ReadString();
        }
        else
        {
            ushort length = reader->ReadUShort();
            // check for setboolattr with rlGui == true
            if (FourCC('SBOA') == curFourCC)
            {
                String attrName = reader->ReadString();
                bool value = reader->ReadBool();
                if (attrName == "rlGui")
                {
                    isGuiScene = value;                       
                }
            }
            else
            {     
                // пропустить не bool attr
                reader->GetStream()->Seek(length, Stream::Current);
            }
        }
    }
    // сбросить позицию потока
    reader->GetStream()->Seek(oldPos, Stream::Begin);

    return isGuiScene;
}

//------------------------------------------------------------------------------
/**
*/
void 
N2Converter::ConvertGuiScene(const Ptr<IO::BinaryReader>& reader, const Ptr<ModelWriter>& writer)
{           
    //n_printf("Преобразование GuiScene %s: \n", this->srcPath.AsCharPtr());

    // создайте один nvx2reader для чтения UV-координации из файла сетки
    this->nvx2Reader = Legacy::Nvx2OrderFreeStreamReader::Create();
    if (this->platform != Platform::Linux
        && this->platform != Platform::Win32)        
    {
        this->nvx2Reader->SetSourcePlatformEndianess(System::ByteOrder::BigEndian);
    }

    const FourCC newFourCC('_new');
    const FourCC selFourCC('_sel');
    FourCC fourCC;
    String objClass;
    String objName;

    Math::bbox curRawLocalBox;

    uint curLevel = 0;
    Util::String meshPath;
    Util::Stack<uint> guiLevel;
    Util::SimpleTree<GuiElement>::Node* node = &this->windowTree.Root();       
    while (!reader->Eof())
    {
        fourCC = reader->ReadUInt();   
        if (fourCC == newFourCC)
        {
            // новый узел обнаружил пропуск строк
            objClass = reader->ReadString();
            String curObjName = reader->ReadString();
            if (objClass == "ntransformnode")
            {
                objName = curObjName;                 
            }      
            //n_printf("ИмяОбъекта: %s \n", curObjName.AsCharPtr());
            curLevel++;            
        }
        else if (fourCC == selFourCC)
        {                   
            String selPath = reader->ReadString();
            
            if (selPath == "..")
            { 
                if (guiLevel.Size() > 0 &&
                    curLevel == guiLevel.Peek())
                {
                    node = &node->Parent();                    
                    guiLevel.Pop();
                }           
                curLevel--;                
            }
        }
        else
        {
            ushort length = reader->ReadUShort();
            // check for setboolattr with rlGui == true
            if (FourCC('SBOA') == fourCC)
            {
                String attrName = reader->ReadString();
                bool value = reader->ReadBool(); 
                if (attrName == "rlGui" && value)
                {    
                    // новый элемент
                    GuiElement newElement;
                    newElement.id = objName;
                    newElement.rawLocalBox = curRawLocalBox;
                    newElement.rect.set(0.0f, 0.0f, 1.0f, 1.0f);
                    node->Append(newElement);

                    // спуститься вниз
                    node = &node->Child(node->Size()-1);
                    guiLevel.Push(curLevel);                    
                }
            }
            else if (FourCC('SLCB') == fourCC)
            {
                // setlocalbox
                float4 mid, extents;
                mid.x() = reader->ReadFloat();
                mid.y() = reader->ReadFloat();
                mid.z() = reader->ReadFloat();                 
                extents.x() = reader->ReadFloat();
                extents.y() = reader->ReadFloat();
                extents.z() = reader->ReadFloat();
                curRawLocalBox.set(mid, extents);
            }
            else if (FourCC('SPOS') == fourCC)
            {
                // позиция чтения
                point pos;
                pos.x() = reader->ReadFloat();
                pos.y() = reader->ReadFloat();
                pos.z() = reader->ReadFloat();
                node->Value().rawTransform.setposition(pos);
            }
            else if (FourCC('SSCL') == fourCC)
            {
                // читать масштаб
                vector scale;
                scale.x() = reader->ReadFloat();
                scale.y() = reader->ReadFloat();
                scale.z() = reader->ReadFloat();
                node->Value().rawTransform.setscale(scale);
            }
            else if (FourCC('SSCP') == fourCC)
            {
                // прочитать ось шкалы
                point scalePivot;
                scalePivot.x() = reader->ReadFloat();
                scalePivot.y() = reader->ReadFloat();
                scalePivot.z() = reader->ReadFloat();
                node->Value().rawTransform.setscalepivot(scalePivot);
            }
            else if (FourCC('SSTA') == fourCC)
            {
                String attrName = reader->ReadString();
                String value = reader->ReadString();      
                // в N3 имя элемента равно имени события 
                if (value != "Generic" && attrName == "rlGuiEvent")
                {                       
                    node->Value().event = value;
                }
                else if (attrName == "rlGuiType")
                {
                    node->Value().guiType = value;                    
                }
            }
            else if (FourCC('STXT') == fourCC)
            {
                String semantic = reader->ReadString();
                String texPath = reader->ReadString();
                if (semantic == "DiffMap0" && node->HasParent())
                {                               
                    texPath.SubstituteString("textures:", "");
                    texPath.StripFileExtension();
                    if (objName == "pressed")
                    {
                        node->Value().pressedTexture = texPath;
                    }
                    else if (objName == "disabled")
                    {
                        node->Value().disabledTexture = texPath;
                    }
                    else if (objName == "mouseover")
                    {
                        node->Value().mouseOverTexture = texPath;
                    }
                    else if (objName == "normal")
                    {
                        node->Value().texture = texPath;
                    }
                    else
                    {
                        // установить глобальную текстуру окна
                        node->Parent().Value().texture = texPath;
                    }
                }                   
            }
            else if (FourCC('SMSH') == fourCC)
            {
                meshPath = reader->ReadString();
                meshPath.SubstituteString("meshes:", String(this->mshDir + "/"));
            }
            else if (FourCC('SGRI') == fourCC)
            {
                uint groupIndex = reader->ReadUInt();
                if (objName.IsValid())
                {     
                    // получить ультрафиолетовые координаты
                    Math::float4 uvs = this->ReadUvCoordinates(meshPath, groupIndex);
                    //n_printf("Записать %s uvs: %s, узла %s\n\n", objName.AsCharPtr(), String::FromFloat4(uvs).AsCharPtr(), node->Value().id.AsCharPtr());
                    // сохранить координаты
                    if (objName == "normal")
                    {
                        node->Value().uv[0] = uvs;
                        objName = "";
                    }
                    else if (objName == "mouseover")
                    {
                        node->Value().uv[1] = uvs;
                        objName = "";
                    }
                    else if (objName == "pressed")
                    {
                        node->Value().uv[2] = uvs;
                        objName = "";
                    }
                    else if (objName == "disabled")
                    {
                        node->Value().uv[3] = uvs;
                        objName = "";
                    }         
                }
            }
            else
            {     
                // пропустить не bool attr
                reader->GetStream()->Seek(length, Stream::Current);
            }
        }         
    }   

    if (this->nvx2Reader->IsOpen())
    {
        this->nvx2Reader->Close();
    }
    this->nvx2Reader = 0;

    IoServer* ioServer = IoServer::Instance();
    ioServer->CreateDirectory("dst:ui");
    String xmlPath;
    xmlPath.Format("лето:%s/%s", "ui", this->dstPath.ExtractFileName().AsCharPtr());
    xmlPath.ChangeFileExtension("xml");
    Ptr<Stream> dstStream = ioServer->CreateStream(xmlPath);
    Ptr<XmlWriter> xmlWriter = XmlWriter::Create();
    xmlWriter->SetStream(dstStream);

    if (xmlWriter->Open())
    {
        xmlWriter->BeginNode("Nebula3");
        xmlWriter->SetString("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");

        SimpleTree<GuiElement>::Node& node = this->windowTree.Root();            
        SizeT numChildren = node.Size();
        if (numChildren > 0)
        {                   
            xmlWriter->BeginNode("Window");
            xmlWriter->SetString("title", "Window");
            xmlWriter->SetString("xAlign", "CenterX");
            xmlWriter->SetString("yAlign", "CenterY");
            xmlWriter->SetString("texture", node.Child(0).Value().texture);
            xmlWriter->SetFloat("fadeIn", 0.5f);
            xmlWriter->SetFloat("fadeOut", 0.5f);
            IndexT i;
            for (i = 0; i < numChildren; ++i)
            {
                this->WriteXmlGuiNode(xmlWriter, node.Child(i));
            }                    
            xmlWriter->EndNode();
        }      
        xmlWriter->EndNode();           
        xmlWriter->Close();
    }
    this->windowTree.Root().Clear();
}

//------------------------------------------------------------------------------
/**
*/
Math::float4 
N2Converter::ReadUvCoordinates(const IO::URI& uri, IndexT meshGroup)
{
    float4 uvs(0,0,1,1);
            
    if (!nvx2Reader->GetStream().isvalid()
        || nvx2Reader->GetStream()->GetURI() != uri)
    {
        if (nvx2Reader->IsOpen())
        {
            nvx2Reader->Close();
        }     
        IoServer* ioServer = IoServer::Instance();
        Ptr<Stream> stream = ioServer->CreateStream(uri);
        nvx2Reader->SetStream(stream);
        nvx2Reader->SetUsage(Base::ResourceBase::UsageCpu);
        nvx2Reader->SetAccess(Base::ResourceBase::AccessRead);
        nvx2Reader->SetRawMode(true);
        bool success = nvx2Reader->Open();     
        n_assert(success);
    }

    System::ByteOrder byteOrder(nvx2Reader->GetSourcePlatformEndianess(), System::ByteOrder::Host);
    
    // получить УФ
    const Util::Array<CoreGraphics::PrimitiveGroup>& primitiveGroups = nvx2Reader->GetPrimitiveGroups();
    n_assert(meshGroup < primitiveGroups.Size());
    IndexT baseIndex = primitiveGroups[meshGroup].GetBaseIndex();
    n_assert(primitiveGroups[meshGroup].GetNumIndices() >= 6);
    ushort* indexPtr = nvx2Reader->GetIndexData();
    char* vertexPtr = (char*)nvx2Reader->GetVertexData();
    ushort uvIndex0 = byteOrder.Convert(indexPtr[baseIndex + 0]);
    ushort uvIndex5 = byteOrder.Convert(indexPtr[baseIndex + 5]);
    const Util::Array<CoreGraphics::VertexComponent>& components = nvx2Reader->GetVertexComponents();
    SizeT texCoordSize = 0;
    SizeT texCoordOffset = 0;  
    SizeT vertexWidth = 0;      
    IndexT i;
    for (i = 0; i < components.Size(); ++i)
    {
        if (texCoordSize == 0)
        {
            if (components[i].GetSemanticName() != CoreGraphics::VertexComponent::TexCoord1
                || components[i].GetSemanticIndex() != 0)           
            {                                                              
                texCoordOffset += components[i].GetByteSize();
            }
            else
            {
                texCoordSize = components[i].GetByteSize() / 2;                
            }
        }
        vertexWidth += components[i].GetByteSize();
    }         
    char* vertex0 = vertexPtr + uvIndex0 * vertexWidth;
    char* vertex5 = vertexPtr + uvIndex5 * vertexWidth;
    n_assert(texCoordOffset > 0 && texCoordSize > 0);

    uvs.x() = (float)byteOrder.Convert(*(ushort*)(vertex0 + texCoordOffset)) / 8192.0f;
    uvs.y() = (float)byteOrder.Convert(*(ushort*)(vertex5 + texCoordOffset + texCoordSize))/ 8192.0f;

    uvs.z() = (float)byteOrder.Convert(*(ushort*)(vertex5 + texCoordOffset))/ 8192.0f;
    uvs.w() = (float)byteOrder.Convert(*(ushort*)(vertex0 + texCoordOffset + texCoordSize))/ 8192.0f;

    return uvs;
}

//------------------------------------------------------------------------------
/**
    Update the position and size members of a GUINode from
    the rawLocalBox, rawPosition and rawSize.
*/
void
N2Converter::ComputeGUINodeDimensions(SimpleTree<GuiElement>::Node& node)
{
    // необходимо вычислить преобразование плоской родительской иерархии
    matrix44 m = node.Value().rawTransform.getmatrix();
    SimpleTree<GuiElement>::Node* nodePtr = &node;
    while (nodePtr->HasParent())
    {
        m = matrix44::multiply(m, nodePtr->Parent().Value().rawTransform.getmatrix());
        nodePtr = &(nodePtr->Parent());
    }

    // трансформировать локальную ограничивающую рамку
    bbox box = node.Value().rawLocalBox;
    box.transform(m);

    // n2 gui имеет диапазон 4:3 от x: -2 -> 2
    //                       and y: -1.5 -> 1.5
    // масштаб влево/вверх: 0 -> вправо/вниз: 1,0

    node.Value().rect.left   = (box.pmin.x() / 4.0f) + 0.5f;
    node.Value().rect.right  = (box.pmax.x() / 4.0f) + 0.5f;
    node.Value().rect.top    = 1.0f - ((box.pmax.y() / 3.0f) + 0.5f);
    node.Value().rect.bottom = 1.0f - ((box.pmin.y() / 3.0f) + 0.5f);
}

//------------------------------------------------------------------------------
/**
*/
void 
N2Converter::WriteXmlGuiNode(const Ptr<XmlWriter>& writer, Util::SimpleTree<GuiElement>::Node& node)
{
    // вычислить размеры узла
    this->ComputeGUINodeDimensions(node);

    // записать в XML-поток
    writer->BeginNode(this->MapGuiType(node.Value().guiType));
    writer->SetString("id", node.Value().id);     
    if (node.Value().event.IsValid())
    {
        writer->SetString("event", node.Value().event);
    }
    float4 r;
    r.x() = node.Value().rect.left;
    r.y() = node.Value().rect.top;
    r.z() = node.Value().rect.right;
    r.w() = node.Value().rect.bottom;
    writer->SetFloat4("rect", r);
    writer->SetFloat4("defaultUV", node.Value().uv[0]);
    if (node.Value().texture.IsValid())
    {
        writer->SetString("texture", node.Value().texture);
    }
    if (node.Value().guiType == "Canvas")
    {
        // поддерживается, но без дополнительных параметров
    }
    else if (node.Value().guiType == "Frame")
    {
        // поддерживается, но без дополнительных параметров
    }
    else if (node.Value().guiType == "Button")
    {      
        writer->SetFloat4("mouseOverUV", node.Value().uv[1]);
        writer->SetFloat4("pressedUV", node.Value().uv[2]);
        writer->SetFloat4("disabledUV", node.Value().uv[3]); 
        if (node.Value().pressedTexture.IsValid())
        {
            writer->SetString("pressedTexture", node.Value().pressedTexture);
        }
        if (node.Value().disabledTexture.IsValid())
        {
            writer->SetString("disabledTexture", node.Value().disabledTexture);
        }
        if (node.Value().mouseOverTexture.IsValid())
        {
            writer->SetString("mouseOverTexture", node.Value().disabledTexture);
        }
    }
    else
    {
        // узел не поддерживается
    }

    SizeT numChildren = node.Size();
    IndexT i;
    for (i = 0; i < numChildren; ++i)
    {
        this->WriteXmlGuiNode(writer, node.Child(i));
    }
    writer->EndNode();
}

//------------------------------------------------------------------------------
/**
*/
Util::String 
N2Converter::MapGuiType(const Util::String& n2Type)
{
    if (n2Type == "Canvas")
    {
        return "Canvas";
    }
    else if (n2Type == "Frame")
    {
        return "Frame";
    }
    else if ((n2Type == "Button") || (n2Type == "TextButton") || (n2Type == "CheckButton"))
    {
        return "Button";
    }
    else if ((n2Type == "Label") || (n2Type == "TextLabel"))
    {
        return "Label";
    }
    else
    {
        return "Frame";
    }
}

//------------------------------------------------------------------------------
/**
*/
N2Converter::GuiElement::GuiElement()
{
    this->rect.set(0.0f, 0.0f, 0.0f, 0.0f);
    uv[0] = Math::vector::nullvec();
    uv[1] = Math::vector::nullvec();
    uv[2] = Math::vector::nullvec();
    uv[3] = Math::vector::nullvec();
}
} // пространство имен ToolkitUtil