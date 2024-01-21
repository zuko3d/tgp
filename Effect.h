#pragma once

enum class EffectType : int {
    GiveResource, // p1 = resource idx, p2 = amount; Could give negative amount
    Terraform, // p1 = amount
    GetTechTile,
    CopyFed,
    
};

struct Effect
{
    EffectType type;
    int param1, param2;
};

