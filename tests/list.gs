# Immutable linked list implementation
using core

type Node(val: int, other: option<Node>) {
    @Getter
    let value = val
    @Getter
    let next = other
}

type LinkedList(val: option<Node>) {
    @Getter
    let head = val

    func prepend(elem: int) -> LinkedList {
        return LinkedList(Some(Node(elem, head)))
    }

    func tail() -> LinkedList {
        if head.isSome() {
            return LinkedList(head.unwrap().getNext())
        }
        return LinkedList(None<Node>)
    }

    func reverse_acc(acc: LinkedList) -> LinkedList {
        if head.isNone() {
            return acc
        }
        let hd = head.unwrap().getValue()
        return tail().reverse_acc(acc.prepend(hd))
    }

    func reverse() -> LinkedList {
        return reverse_acc(LinkedList(None<Node>))
    }

    func to_array() -> int[] {
        let mut out = [::int]
        let mut current = head
        while current.isSome() {
            let raw = current.unwrap()
            out := out.add(raw.getValue())
            current := raw.getNext()
        }
        return out
    }

    func length() -> int {
        if head.isSome() {
            return 1 + tail().length()
        }
        return 0
    }
}

func List() -> LinkedList {
    return LinkedList(None<Node>)
}

func printList(list: LinkedList) {
    let mut current = list.getHead()
    print("( ")
    while current.isSome() {
        let raw = current.unwrap()
        let value = raw.getValue()
        print("$value ")
        current := raw.getNext()
    }
    println([")"])
}
