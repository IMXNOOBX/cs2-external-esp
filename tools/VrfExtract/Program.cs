using System.Numerics;
using System.Runtime.InteropServices;
using ValveResourceFormat;
using ValveResourceFormat.ResourceTypes;
using ValveResourceFormat.ResourceTypes.RubikonPhysics;
using ValveResourceFormat.Serialization.KeyValues;

// === Entry point (must precede type declarations in C# top-level) ===
if (args.Length < 3 || (args[0] != "--vpk" && args[0] != "--vmdl"))
{
    Console.Error.WriteLine("Usage: VrfExtract --vpk <map.vpk> <output.tri>");
    Console.Error.WriteLine("       VrfExtract --vmdl <world_physics.vmdl_c> <output.tri>");
    return 1;
}

var mode = args[0];
var inputPath = args[1];
var outputPath = args[2];

byte[]? inputBytes;
if (mode == "--vpk")
{
    var mapName = Path.GetFileNameWithoutExtension(inputPath);
    inputBytes = VpkReader.ReadEntry(inputPath, $"maps/{mapName}/world_physics.vmdl_c");
    if (inputBytes == null) { Console.Error.WriteLine("Entry not found"); return mapName.EndsWith("_vanity") ? 0 : 1; }
}
else
{
    inputBytes = File.ReadAllBytes(inputPath);
    if (inputBytes.Length == 0) return 1;
}

var triangles = PhysParser.Parse(inputBytes);
if (triangles == null || triangles.Count == 0) { Console.Error.WriteLine("No triangles"); return 1; }

Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(outputPath)) ?? ".");
var span = CollectionsMarshal.AsSpan(triangles);
File.WriteAllBytes(outputPath, MemoryMarshal.AsBytes(span).ToArray());

var mapLabel = mode == "--vpk" ? Path.GetFileNameWithoutExtension(inputPath) : Path.GetFileName(inputPath);
Console.WriteLine($"[OK] {mapLabel}: {triangles.Count} triangles");
return 0;

// === Helper types ===

static class VpkReader
{
    public static byte[]? ReadEntry(string vpkPath, string targetEntry)
    {
        using var fs = File.OpenRead(vpkPath);
        using var br = new BinaryReader(fs);

        uint sig = br.ReadUInt32();
        if (sig != 0x55AA1234) throw new InvalidDataException("Not VPK v2");
        uint ver = br.ReadUInt32();
        if (ver != 2) throw new InvalidDataException($"VPK v{ver} unsupported");
        uint treeSize = br.ReadUInt32();
        br.ReadUInt32(); br.ReadUInt32(); br.ReadUInt32(); br.ReadUInt32();

        long treeStart = fs.Position, treeEnd = treeStart + treeSize;

        string targetExt = Path.GetExtension(targetEntry).TrimStart('.');
        string targetPath = Path.GetDirectoryName(targetEntry)!.Replace('\\', '/');
        string targetFile = Path.GetFileNameWithoutExtension(targetEntry);

        while (fs.Position < treeEnd)
        {
            string ext = ReadCStr(br); if (ext == "") break;
            while (fs.Position < treeEnd)
            {
                string path = ReadCStr(br); if (path == "") break;
                while (fs.Position < treeEnd)
                {
                    string file = ReadCStr(br); if (file == "") break;
                    uint crc = br.ReadUInt32();
                    ushort preloadBytes = br.ReadUInt16();
                    ushort archiveIndex = br.ReadUInt16();
                    uint entryOffset = br.ReadUInt32();
                    uint entryLength = br.ReadUInt32();
                    ushort terminator = br.ReadUInt16();
                    if (terminator != 0xFFFF) throw new InvalidDataException("Bad terminator");

                    if (ext == targetExt && path == targetPath && file == targetFile)
                    {
                        var data = new byte[entryLength];
                        if (archiveIndex == 0x7FFF)
                        {
                            fs.Seek(treeStart + treeSize + preloadBytes + entryOffset, SeekOrigin.Begin);
                            fs.ReadExactly(data, 0, (int)entryLength);
                        }
                        else
                        {
                            string dirFile = vpkPath[..^4] + "_dir.vpk";
                            if (!File.Exists(dirFile)) return null;
                            using var dfs = File.OpenRead(dirFile);
                            dfs.Seek(preloadBytes + entryOffset, SeekOrigin.Begin);
                            dfs.ReadExactly(data, 0, (int)entryLength);
                        }
                        return data;
                    }
                }
            }
        }
        return null;
    }

    static string ReadCStr(BinaryReader br)
    {
        var chars = new List<char>();
        char c;
        while ((c = br.ReadChar()) != '\0') chars.Add(c);
        return new string([.. chars]);
    }
}

[StructLayout(LayoutKind.Sequential)]
struct Tri(Vector3 v0, Vector3 v1, Vector3 v2)
{
    public Vector3 V0 = v0, V1 = v1, V2 = v2;
}

static class PhysParser
{
    public static List<Tri>? Parse(byte[] vmdlData)
    {
        var resource = new Resource();
        using (var ms = new MemoryStream(vmdlData))
            resource.Read(ms);

        PhysAggregateData? phys = resource.DataBlock as PhysAggregateData;
        if (phys == null)
        {
            foreach (var block in resource.Blocks)
                if (block is PhysAggregateData p) { phys = p; break; }
        }
        if (phys == null) return null;

        var collisionAttrs = phys.CollisionAttributes;
        var defaultCol = new HashSet<int>();
        for (int i = 0; i < collisionAttrs.Count; i++)
        {
            var group = collisionAttrs[i].GetStringProperty("m_CollisionGroupString") ?? "";
            if (string.IsNullOrEmpty(group) || group.Equals("default", StringComparison.OrdinalIgnoreCase))
                defaultCol.Add(i);
        }
        if (defaultCol.Count == 0 && collisionAttrs.Count > 0) defaultCol.Add(0);

        var tris = new List<Tri>();
        var parts = phys.Parts;
        var bindPose = phys.BindPose;

        for (int p = 0; p < parts.Length; p++)
        {
            var shape = parts[p].Shape;
            var pose = p < bindPose.Length ? bindPose[p] : Matrix4x4.Identity;

            foreach (var hull in shape.Hulls)
            {
                if (!defaultCol.Contains(hull.CollisionAttributeIndex)) continue;
                var hullVerts = hull.Shape.GetVertexPositions();
                if (hullVerts.Length == 0) continue;
                var verts = new Vector3[hullVerts.Length];
                for (int i = 0; i < hullVerts.Length; i++)
                    verts[i] = Vector3.Transform(hullVerts[i], pose);

                var faces = hull.Shape.GetFaces();
                var edges = hull.Shape.GetEdges();
                foreach (var face in faces)
                {
                    int start = face.Edge;
                    for (int e = edges[start].Next; e != start;)
                    {
                        int n = edges[e].Next;
                        if (n == start) break;
                        tris.Add(new Tri(verts[edges[start].Origin], verts[edges[e].Origin], verts[edges[n].Origin]));
                        e = n;
                    }
                }
            }

            foreach (var mesh in shape.Meshes)
            {
                if (!defaultCol.Contains(mesh.CollisionAttributeIndex)) continue;
                var meshVerts = mesh.Shape.GetVertices();
                if (meshVerts.Length == 0) continue;
                var verts = new Vector3[meshVerts.Length];
                for (int i = 0; i < meshVerts.Length; i++)
                    verts[i] = Vector3.Transform(meshVerts[i], pose);

                foreach (var tri in mesh.Shape.GetTriangles())
                {
                    if (tri.X < 0 || tri.Y < 0 || tri.Z < 0) continue;
                    if (tri.X >= verts.Length || tri.Y >= verts.Length || tri.Z >= verts.Length) continue;
                    tris.Add(new Tri(verts[(int)tri.X], verts[(int)tri.Y], verts[(int)tri.Z]));
                }
            }
        }
        return tris;
    }
}