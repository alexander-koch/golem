# TODO-List Implementation
# Copyright (c) Alexander Koch 2015
using core

type Task(_title: str, _desc: str, _done: bool) {
	@Getter
	let title = _title
	@Getter
	let description = _desc
	@Getter
	let done = _done
}

type TodoList(tasks: Task[]) {
	let taskList = tasks

	func printTasks() {
		let mut i = 0
		while i < taskList.length() {
			let task = taskList.at(i)
			print("Task: ")
			println(task.getTitle())
			print("Description: ")
			println(task.getDescription())
			print("Done?: ")
			println(task.getDone())
			print(["\n"])
			i := i + 1
		}
	}
}

let t1 = Task("S.P.E.C.I.A.L Task", "Upgrade your abilities", false)
let t2 = Task("Todo-List", "Run this example", true)
let list = TodoList([t1, t2])
list.printTasks()
