If you cannot run this project at a playable framerate you can change the number of instances by adjusting the scene entry logic in RenderApplication.cpp
```cpp
    Scene::SceneEntry entry = {};
    entry.model_handle = sponza;
    entry.transform_index = scene.all_transforms.size();
    entry.transform_count = 0;

    for (int x = 1; x < 15; x++) {
        for (int z =1; z < 15; z++) {
            auto transform = Transform{};
            transform.SetPosition({x*5000,0,z*5000});
            transform.SetScale({1,1,1});

            scene.all_transforms.push_back(transform);
            entry.transform_count++;
        }
    }
    scene.model_entries.push_back(entry);

    Scene::SceneEntry entry_two = {};
    entry_two.model_handle = sponza_two;
    entry_two.transform_index = scene.all_transforms.size();
    entry_two.transform_count = 0;
    for (int x = 1; x < 15; x++) {
        for (int z = 1; z < 15; z++) {
            auto transform = Transform{};
            transform.SetPosition({-x*5000,0,-z*5000});
            transform.SetScale({1,1,1});
            scene.all_transforms.push_back(transform);
            entry_two.transform_count++;
        }
    }
    scene.model_entries.push_back(entry_two);

```
