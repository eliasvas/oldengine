#ifndef ENTITY_H
#define ENTITY_H
#include "tools.h"
#include "platform.h"

#define MAX_ENTITY 1024
#define MAX_COMPONENTS 1024
typedef u32 entity_id;
typedef u32 Entity;
#define INVALID_ENTITY 0

typedef IntHashMap EntityHashMap;
typedef struct PositionComponent
{
   vec3 position; 
}PositionComponent;

typedef struct PositionManager
{
    PositionComponent positions[MAX_COMPONENTS];
    Entity entities[MAX_ENTITY];
    u32 next_index;
    EntityHashMap table;
}PositionManager;

//creates an empty component of type position TODO generalize?
internal PositionComponent*
entity_add_pos(PositionManager *manager, Entity entity)
{
  // INVALID_ENTITY is not allowed!
  assert(entity != INVALID_ENTITY);

  // Only one of this component type per entity is allowed!
  //assert(lookup_hashmap(&manager->table, entity) == -1);

  // Update the entity lookup table:
  hashmap_insert(&manager->table, entity, manager->next_index);

  // New components are always pushed to the end:
  manager->positions[manager->next_index] = (PositionComponent){0}; 

  // Also push corresponding entity:
  manager->entities[manager->next_index] = entity;

  return &manager->positions[manager->next_index++];
}

//removes an entity and its component from the manager's data TODO generalize?
internal void 
entity_remove_pos(PositionManager* manager, Entity entity)
{
  u32 index = hashmap_lookup(&manager->table, entity);
  if (index != -1)
  {
    // Directly index into components and entities array:
    Entity entity = manager->entities[index];

    if (index < manager->next_index)
    {
      // Swap out the dead element with the last one:
      manager->positions[index] = manager->positions[manager->next_index-1];// try to use move
      manager->entities[index] = manager->entities[manager->next_index-1];

      //NOTE: this is not ver performant..
      hashmap_remove(&manager->table,entity);
      hashmap_remove(&manager->table,manager->entities[index]);
      hashmap_insert(&manager->table,manager->entities[index], index); 
    }

    //shrink the container 
    manager->next_index--;
  }
}

internal PositionComponent* 
entity_get_pos(PositionManager *manager, Entity entity)
{
    i32 index = hashmap_lookup(&manager->table, entity);
    if (index != -1)
    {
        return &manager->positions[index];
    }
    return NULL;
}
internal void
position_manager_init(PositionManager *manager)
{
    manager->table = hashmap_create(20);
    manager->next_index = 0;
}



typedef struct Model ModelComponent;


typedef struct ModelManager
{
    ModelComponent models[MAX_COMPONENTS];
    Entity entities[MAX_ENTITY];
    u32 next_index;
    EntityHashMap table;
}ModelManager;

internal ModelComponent*
entity_add_model(ModelManager *manager, Entity entity)
{
  assert(entity != INVALID_ENTITY);
  hashmap_insert(&manager->table, entity, manager->next_index);
  manager->models[manager->next_index] = (ModelComponent){0}; 
  manager->entities[manager->next_index] = entity;
  return &manager->models[manager->next_index++];
}

internal void 
entity_remove_model(ModelManager* manager, Entity entity)
{
  u32 index = hashmap_lookup(&manager->table, entity);
  if (index != -1)
  {
    Entity entity = manager->entities[index];
    if (index < manager->next_index)
    {
      manager->models[index] = manager->models[manager->next_index-1];// try to use move
      manager->entities[index] = manager->entities[manager->next_index-1];
      hashmap_remove(&manager->table,entity);
      hashmap_remove(&manager->table,manager->entities[index]);
      hashmap_insert(&manager->table,manager->entities[index], index); 
    }
    manager->next_index--;
  }
}

internal ModelComponent* 
entity_get_model(ModelManager *manager, Entity entity)
{
    i32 index = hashmap_lookup(&manager->table, entity);
    if (index != -1)
    {
        return &manager->models[index];
    }
    return NULL;
}
internal void
model_manager_init(ModelManager *manager)
{
    manager->table = hashmap_create(20);
    manager->next_index = 0;
}

typedef struct EntityManager
{
    Entity next_entity;
    PositionManager position_manager;
    ModelManager model_manager;
}EntityManager;


internal void 
entity_manager_init(EntityManager *manager)
{
    manager->next_entity = 0;
    position_manager_init(&manager->position_manager);
    model_manager_init(&manager->model_manager);
}
internal Entity 
entity_create(EntityManager *manager)
{
   return ++manager->next_entity; 
}
/*
*/

internal i32 last_entity_pressed = -1;
internal void 
entity_manager_update(EntityManager *manager, Renderer *rend)
{
    Ray r =  (Ray){rend->cam.pos, 1, v3(0,0,0)};
    r.d = get_ray_dir(v2(global_platform.mouse_x, global_platform.mouse_y),global_platform.window_width, global_platform.window_height, rend->view, rend->proj);//(Ray){v3(0,0,0), 1, v3(0,0,-1)};
    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        mat4 model = manager->model_manager.models[i].model;
        vec3 pos = v3(model.elements[3][0], model.elements[3][1], model.elements[3][2]);
        i32 collision = intersect_ray_sphere_simple(r, (Sphere){pos, 0.5});
        if (collision && global_platform.right_mouse_down || last_entity_pressed == i)
        {
            //sprintf(error_log, "good collision, mouse_dt = %f, %f", global_platform.mouse_dt.x, global_platform.mouse_dt.y);
            last_entity_pressed = i;
            vec3 right = vec3_normalize(vec3_cross(rend->cam.front, rend->cam.up));
            vec3 up = vec3_normalize(rend->cam.up);
            manager->model_manager.models[i].model.elements[3][0] += vec3_mulf(right, (-20.f *(f32)global_platform.mouse_dt.x / global_platform.window_width)).x;
            manager->model_manager.models[i].model.elements[3][1] += vec3_mulf(up,((f32)20.f * global_platform.mouse_dt.y / global_platform.window_height)).y;
            //sprintf(error_log, "collision detected!!");
        }
        else if (global_platform.right_mouse_down && last_entity_pressed == i) {
            last_entity_pressed = i;
            //sprintf(error_log, "collision detected!!");
        }
        else if (!global_platform.right_mouse_down)
            last_entity_pressed = -1;
    }

}
internal void 
entity_manager_render(EntityManager *manager, Renderer *rend)
{
    if (last_entity_pressed > 0)
    {
        mat4 model = manager->model_manager.models[last_entity_pressed].model;
        vec3 pos = v3(model.elements[3][0], model.elements[3][1], model.elements[3][2]);
        renderer_push_compass(rend, pos);
    }
    for (u32 i = 0; i < manager->model_manager.next_index; ++i)
    {
        renderer_push_model(rend, manager->model_manager.models[i]);
    }
}

#include "stdio.h"
void scene_init(char *filepath, EntityManager * manager)
{
    FILE *file = fopen(filepath, "r");
    if (!file)return;
    Model *m;
    char str[256];
    vec3 pos;
    vec3 scale;
    f32 angle;
    vec3 axis;
    while(TRUE)
    {
        i32 res = fscanf(file, "%s", str);
        if (res == EOF || res == 0){fclose(file);return;}
        fscanf(file,"%f %f %f", &pos.x, &pos.y, &pos.z);
        fscanf(file,"%f %f %f", &scale.x, &scale.y, &scale.z);
        if (strcmp("CUBE", str) == 0)
        {
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            model_init_cube(m);
            m->model = mat4_mul(mat4_translate(pos), mat4_mul(mat4_rotate(40, v3(1,1,0)), mat4_scale(scale)));
        }
        else if (strcmp("SPHERE", str) == 0)
        {
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            model_init_sphere(m, 1, 20, 20);
            m->model = mat4_mul(mat4_translate(pos), mat4_scale(scale));
            //sprintf(error_log, "sphere done");
        }
        else
        {
            fscanf(file,"%f", &angle);
            fscanf(file,"%f %f %f", &axis.x, &axis.y, &axis.z);
            m = entity_add_model(&manager->model_manager,entity_create(manager));
            (*m) = model_info_init(str);
            (*m).model = mat4_mul(mat4_translate(pos),mat4_mul(mat4_rotate(angle, axis), mat4_scale(scale)));
        }

    }

    fclose(file);
}
#endif
