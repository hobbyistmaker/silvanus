import logging
from collections import defaultdict
from collections import namedtuple

# noinspection SpellCheckingInspection
logger = logging.getLogger('silvanus.lib.generatebox.ecs')


class ComponentQuery:

    def __init__(self, components):
        self._components = components

    def for_each(self, func):
        for component in self._components:
            func(component)

    def with_all(self, func):
        func(self._components)

    def with_true(self, filter_fun):
        return ComponentQuery(filter(filter_fun, self._components))

    @property
    def instances(self):
        return self._components


class Entity:
    em = None

    def __init__(self, id_):
        self.id = id_

    def add_component(self, component):
        self.em.add_component(self.id, component)
        return self

    def add_components(self, *components):
        self.em.add_components(self.id, *components)
        return self

    def remove_component(self, component):
        self.em.remove_component(self.id, component)
        return self


class Repository:
    axes = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))
    names = defaultdict(lambda: defaultdict(lambda: defaultdict(list)))
    input_panels = { }
    defined_panels = { }
    thickness_groups = defaultdict(lambda: defaultdict(lambda: defaultdict(lambda: defaultdict(list))))
    parameters = { }
    enabled = { }
    _entity_remove_buffer = []

    enable = { }
    override = { }
    thickness = { }
    dimensions = { }
    axis = { }

    _components = defaultdict(dict)
    _entities = defaultdict(set)

    orientation = 0
    root = None
    preview = False
    defined = False

    _next_entity_id = 0

    def create(self, *components):
        self._next_entity_id += 1
        entity = self._next_entity_id

        for component in components:
            component_type = type(component)
            self._components[component_type][entity] = component
            self._entities[entity].add(component_type)

        new_entity = Entity(entity)
        new_entity.em = self
        return new_entity

    def add_component(self, entity, component):
        component_type = type(component)
        logger.debug(f'Adding {component_type} to {entity}')
        self._components[component_type][entity] = component
        self._entities[entity].add(component)

    def add_components(self, entity, *components):
        for component in components:
            component_type = type(component)
            logger.debug(f'Adding {component_type} to {entity}')
            self._components[component_type][entity] = component
            self._entities[entity].add(component_type)

    def with_components(self, *component_types):
        all_components = self._components

        component_names = [
            component_type.__name__
            for component_type in component_types
        ]

        EntityQuery = namedtuple('EntityQuery', ['id', *component_names])

        class EntityResult(EntityQuery):
            em = self

            def add_component(s, component):
                s.em.add_component(s.id, component)
                return s

            def add_components(s, *components):
                s.em.add_components(s.id, *components)
                return s

            def remove_component(s, component):
                s.em.remove_component(s.id, component)
                return s

        components = [
            EntityResult(entity, *[all_components[component_type][entity] for component_type in component_types])
            for entity in set.intersection(*[
                set(all_components[component_type])
                for component_type in component_types
            ])
        ]
        return ComponentQuery(components)

    def remove_entity(self, entity):
        self._entity_remove_buffer.append(entity)

    def remove_component(self, entity, component_type):
        if component_type not in self._components:
            return False

        del self._components[component_type][entity]
        if not self._components[component_type]:
            del self._components[component_type]

        if not self._entities[entity]:
            del self._entities[entity]

        return True

    def _remove_entity_immediate(self, entity):
        self._entities[entity].clear()
        del self._entities[entity]

    def flush(self):
        for entity in self._entity_remove_buffer:
            for component_type in self._entities[entity]:
                self.remove_component(entity, component_type)

            self._remove_entity_immediate(entity)

        self._entity_remove_buffer.clear()

    def clear(self):
        self.axes.clear()
        self.names.clear()
        self.thickness_groups.clear()
        self.enabled.clear()
        self.preview = False
        self.root = None
        self.orientation = 0
        self.parameters.clear()
        self.defined_panels.clear()
        self._next_entity_id = 0
        self._components.clear()
        self._entities.clear()


class Process:
    priority = 0
    _config = None
    _repository = None


class ProcessManager(Process):

    def __init__(self):
        self._processes = []

        self._post_init_()

    def _post_init_(self):
        pass

    def create_process(self, process, *args, **kwargs):
        instance = process(*args, **kwargs)
        self._processes.append(instance)
        return instance

    def add_process(self, process, priority=0):
        self.create_process(process, priority)
        return self

    def process(self, *args, **kwargs):
        self._process(*args, **kwargs)

    def _process(self, *args, **kwargs):
        for process in self._processes:
            process._config = self._config
            process._repository = self._repository
            process.process(*args, **kwargs)
            self._repository.flush()

    def clear(self):
        self._processes.clear()


