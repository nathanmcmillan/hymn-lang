examples.set(
  'new',
  `function new_node(value)
  return { value: value, next: none }
end

function node_add(list, value)
  let node = list
  while true
    if node.next == none break end
    node = node.next
  end
  node.next = new_node(value)
end

const list = new_node("hello")
node_add(list, "world")

iterate i, value in list
  print value
end`
)
