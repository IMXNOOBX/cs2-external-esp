using System.Numerics;
using System.Runtime.InteropServices;
using ValveKeyValue;
using ValveResourceFormat;
using ValveResourceFormat.ResourceTypes;
using ValveResourceFormat.ResourceTypes.RubikonPhysics;
using ValveResourceFormat.Serialization.KeyValues;
using static ValveResourceFormat.ResourceTypes.RubikonPhysics.Shapes.Hull;
using static ValveResourceFormat.ResourceTypes.RubikonPhysics.Shapes.Mesh;

if (args.Length < 2) {
    Console.Error.WriteLine("Usage: VrfExtract <world_physics.vmdl_c> <output.tri>");
    return 1;
}
var input = args[0];
var output = args[1];
if (!File.Exists(input)) {
    Console.Error.WriteLine($"Input not found: {input}");
    return 1;
}
using var stream = File.OpenRead(input);
var resource = new Resource();
resource.Read(stream);
var phys = resource.DataBlock as PhysAggregateData;
if (phys == null) {
    foreach (var block in resource.Blocks) {
        if (block is PhysAggregateData p) {
            phys = p;
            break;
        }
    }
}
if (phys == null) {
    Console.Error.WriteLine("No PHYS block in resource");
    return 1;
}
var defaultCollision = DefaultCollisionIndices(phys.CollisionAttributes);
var tris = new List<TriangleCombined>();
for (var p = 0; p < phys.Parts.Length; p++) {
    var shape = phys.Parts[p].Shape;
    var pose = phys.BindPose.Length == 0 ? Matrix4x4.Identity : phys.BindPose[p];
    foreach (var hull in shape.Hulls) {
        if (!defaultCollision.Contains(hull.CollisionAttributeIndex))
            continue;
        var verts = Transform(hull.Shape.GetVertexPositions(), pose);
        TriangulateHull(hull.Shape.GetFaces(), hull.Shape.GetEdges(), verts, tris);
    }
    foreach (var mesh in shape.Meshes) {
        if (!defaultCollision.Contains(mesh.CollisionAttributeIndex))
            continue;
        var verts = Transform(mesh.Shape.GetVertices(), pose);
        foreach (var tri in mesh.Shape.GetTriangles()) {
            if (tri.X < 0 || tri.Y < 0 || tri.Z < 0)
                continue;
            if (tri.X >= verts.Length || tri.Y >= verts.Length || tri.Z >= verts.Length)
                continue;
            tris.Add(new TriangleCombined(verts[tri.X], verts[tri.Y], verts[tri.Z]));
        }
    }
}

if (tris.Count == 0) {
    Console.Error.WriteLine("No triangles extracted");
    return 1;
}

Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(output)) ?? ".");
File.WriteAllBytes(output, MemoryMarshal.AsBytes(CollectionsMarshal.AsSpan(tris)));
Console.Error.WriteLine($"Wrote {tris.Count} triangles ({tris.Count * 36L} bytes)");
return 0;

static HashSet<int> DefaultCollisionIndices(IReadOnlyList<KVObject> attrs) {
    var indices = new HashSet<int>();
    for (var i = 0; i < attrs.Count; i++) {
        var group = attrs[i].GetStringProperty("m_CollisionGroupString");
        if (string.IsNullOrEmpty(group) || group.Equals("default", StringComparison.OrdinalIgnoreCase))
            indices.Add(i);
    }
    if (indices.Count == 0 && attrs.Count > 0)
        indices.Add(0);
    return indices;
}

static Vector3[] Transform(ReadOnlySpan<Vector3> src, Matrix4x4 pose) {
    var dst = new Vector3[src.Length];
    for (var i = 0; i < src.Length; i++)
        dst[i] = Vector3.Transform(src[i], pose);
    return dst;
}

static void TriangulateHull(ReadOnlySpan<Face> faces, ReadOnlySpan<HalfEdge> edges, Vector3[] verts, List<TriangleCombined> outTris) {
    foreach (var face in faces) {
        var startEdge = face.Edge;
        for (var edge = edges[startEdge].Next; edge != startEdge;) {
            var next = edges[edge].Next;
            if (next == startEdge)
                break;
            outTris.Add(new TriangleCombined(
                verts[edges[startEdge].Origin],
                verts[edges[edge].Origin],
                verts[edges[next].Origin]));
            edge = next;
        }
    }
}

[StructLayout(LayoutKind.Sequential)]
readonly struct TriangleCombined(Vector3 v0, Vector3 v1, Vector3 v2) {
    public readonly Vector3 V0 = v0;
    public readonly Vector3 V1 = v1;
    public readonly Vector3 V2 = v2;
}

